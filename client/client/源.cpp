#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma execution_character_set("utf-8")

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

class SocketClient {
private:
    SOCKET sock;
public:
    SocketClient() : sock(INVALID_SOCKET) {}
    bool init() const {
        WSADATA wsa;
        return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
    }
    void cleanup() const { WSACleanup(); }
    void closeSocket() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }
   // 功能：连接服务器
// 参数：ip为服务器IP地址，port为服务器端口号
// 返回值：连接成功返回true，否则返回false
    bool connectServer(const string& ip, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        return connect(sock, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR;
    }
    // 功能：向服务器发送一条请求消息
// 参数：msg为待发送的数据
// 返回值：发送成功返回true，否则返回false
    bool sendLine(const string& msg) const {
        string data = msg + "\n";
        const char* p = data.c_str();
        int left = (int)data.size();
        while (left > 0) {
            int n = send(sock, p, left, 0);
            if (n <= 0) return false;
            p += n; left -= n;
        }
        return true;
    }
    // 功能：接收服务器返回的数据
// 参数：response用于保存服务器响应结果
// 返回值：接收成功返回true，否则返回false
    bool readResponse(string& response) const {
        response.clear();
        string line; char ch;
        while (true) {
            line.clear();
            while (true) {
                int n = recv(sock, &ch, 1, 0);
                if (n <= 0) return false;
                if (ch == '\n') break;
                if (ch != '\r') line.push_back(ch);
            }
            if (line == "END") break;
            response += line + "\n";
        }
        return true;
    }
    // 功能：封装客户端请求流程
// 流程：发送请求 -> 接收响应
// 参数：cmd为请求命令
// 返回值：服务器返回结果
    string request(const string& cmd) const {
        string res;
        if (!sendLine(cmd)) return "FAIL\n发送失败\n";
        if (!readResponse(res)) return "FAIL\n服务器断开\n";
        return res;
    }
};

class ClientApp {
private:
    SocketClient net;
    string role, username;

    string input(const string& prompt) const {
        cout << prompt;
        string s; getline(cin, s);
        return s;
    }
    int inputInt(const string& prompt) const {
        while (true) {
            string s = input(prompt);
            stringstream ss(s);
            int v; char extra;
            if ((ss >> v) && !(ss >> extra)) return v;
            cout << "请输入数字。\n";
        }
    }
    double inputDouble(const string& prompt) const {
        while (true) {
            string s = input(prompt);
            stringstream ss(s);
            double v; char extra;
            if ((ss >> v) && !(ss >> extra) && v > 0) return v;
            cout << "请输入大于0的数字。\n";
        }
    }
    void show(const string& s) const {
        cout << "\n----- 服务器返回 -----\n" << s << "----------------------\n";
    }
    string getExpressIdByNumber(const string& res, int n) {
        if (n <= 0) return "";

        stringstream ss(res);
        string line;
        int count = 0;

        while (getline(ss, line)) {
            // 找快递单号 EX 开头的位置
            size_t pos = line.find("EX");
            if (pos == string::npos) continue;

            // 提取 EX 后面的完整单号
            size_t end = pos;
            while (end < line.size() &&
                (isalnum((unsigned char)line[end]) || line[end] == '_')) {
                end++;
            }

            string id = line.substr(pos, end - pos);

            count++;
            if (count == n) {
                return id;
            }
        }

        return "";
    }
    void showNumberedExpressList(const string& response) const {
        stringstream ss(response);
        string line;
        int count = 0;
        cout << "\n===== 可选择快递列表 =====\n";
        while (getline(ss, line)) {
            if (line.find("单号:") == 0) {
                count++;
                cout << count << ". " << line << "\n";
            }
            else {
                cout << line << "\n";
            }
        }
        if (count == 0) cout << "没有可选择的快递。\n";
    }
    string chooseQueryField(bool includeType, bool includeCourier) const {
        cout << "1. 全部\n";
        cout << "2. 按寄件人\n";
        cout << "3. 按收件人\n";
        cout << "4. 按快递单号\n";
        cout << "5. 按状态\n";
        cout << "6. 按日期\n";
        cout << "7. 按物品描述\n";
        int baseMax = 7;
        if (includeType) { cout << "8. 按快递类型\n"; baseMax = 8; }
        if (includeCourier) { cout << "9. 按快递员\n"; baseMax = 9; }
        int c = inputInt("请选择查询方式:");
        if (c == 1) return "all|";
        string field;
        if (c == 2) field = "sender";
        else if (c == 3) field = "receiver";
        else if (c == 4) field = "id";
        else if (c == 5) field = "status";
        else if (c == 6) field = "date";
        else if (c == 7) field = "desc";
        else if (c == 8 && includeType) field = "type";
        else if (c == 9 && includeCourier) field = "courier";
        else {
            cout << "无效选项，默认查询全部。\n";
            return "all|";
        }
        string key = input("请输入查询关键字:");
        return field + "|" + key;
    }

    void registerUser() {
        string u = input("用户名:");
        string n = input("姓名:");
        string ph = input("电话:");
        string pw = input("密码:");
        string ad = input("地址:");
        show(net.request("REGISTER_USER|" + u + "|" + n + "|" + ph + "|" + pw + "|" + ad));
    }
    bool login(const string& r) {
        string u = input("账号:");
        string p = input("密码:");
        string res = net.request("LOGIN|" + r + "|" + u + "|" + p);
        show(res);
        if (res.find("OK") == 0) {
            role = r; username = u; return true;
        }
        return false;
    }

    void changePassword() {
        string oldp = input("原密码:");
        string newp = input("新密码:");
        string confirm = input("再次输入新密码:");
        if (newp != confirm) {
            cout << "两次密码不一致。\n";
            return;
        }
        show(net.request("CHANGE_PASSWORD|" + username + "|" + oldp + "|" + newp));
    }
    void recharge() {
        double amount = inputDouble("充值金额:");
        show(net.request("RECHARGE|" + username + "|" + to_string(amount)));
    }
    void sendExpress() {
        string receiver = input("收件用户名:");
        string desc = input("物品描述:");
        cout << "1. 易碎品 8元/kg\n";
        cout << "2. 图书 2元/本\n";
        cout << "3. 普通快递 5元/kg\n";
        int type = inputInt("类型:");
        double num = inputDouble(type == 2 ? "本数:" : "重量/kg:");
        show(net.request("SEND_EXPRESS|" + username + "|" + receiver + "|" + desc + "|" + to_string(type) + "|" + to_string(num)));
    }
    void signExpress() {
        string res = net.request("LIST_PENDING_SIGN|" + username);
        showNumberedExpressList(res);
        int n = inputInt("请输入要签收的序号:");
        string id = getExpressIdByNumber(res, n);
        if (id.empty()) cout << "序号无效。\n";
        else show(net.request("SIGN|" + username + "|" + id));
    }
    void queryMyExpress() {
        cout << "\n===== 用户查询快递 =====\n";
        string q = chooseQueryField(true, false);
        if (q == "all|") show(net.request("QUERY_MY|" + username));
        else show(net.request("QUERY_MY_FIELD|" + username + "|" + q));
    }

    void userMenu() {
        while (true) {
            cout << "\n===== 用户菜单 =====\n";
            cout << "1. 修改账户密码\n";
            cout << "2. 查询余额\n";
            cout << "3. 充值\n";
            cout << "4. 发送快递\n";
            cout << "5. 查看待签收\n";
            cout << "6. 签收快递（按序号）\n";
            cout << "7. 查询我的快递\n";
            cout << "8. 评价已签收快递\n";
            cout << "0. 退出登录\n";

            int c = inputInt("请选择:");

            if (c == 1) changePassword();
            else if (c == 2) show(net.request("USER_BALANCE|" + username));
            else if (c == 3) recharge();
            else if (c == 4) sendExpress();
            else if (c == 5) show(net.request("LIST_PENDING_SIGN|" + username));
            else if (c == 6) signExpress();
            else if (c == 7) queryMyExpress();

            // 新增创新点：评价已签收快递
            else if (c == 8) {
                string id = input("请输入已签收快递单号:");
                int score = inputInt("请输入评分1-5:");
                string content = input("请输入评价内容:");

                show(net.request("EVALUATE|" + username + "|" + id + "|" + to_string(score) + "|" + content));
            }

            else if (c == 0) break;
            else cout << "无效选择。\n";
        }
    }

    void courierQuery() {
        cout << "\n===== 快递员任务查询 =====\n";
        string q = chooseQueryField(false, false);
        if (q == "all|") show(net.request("COURIER_QUERY|" + username));
        else show(net.request("COURIER_QUERY_FIELD|" + username + "|" + q));
    }
    void pickupExpress() {
        string res = net.request("COURIER_LIST_WAIT|" + username);
        showNumberedExpressList(res);
        int n = inputInt("请输入要揽收的序号:");
        string id = getExpressIdByNumber(res, n);
        if (id.empty()) cout << "序号无效。\n";
        else show(net.request("COURIER_PICKUP|" + username + "|" + id));
    }
    void courierMenu() {
        while (true) {
            cout << "\n===== 快递员菜单 =====\n";
            cout << "1. 查看待揽收\n";
            cout << "2. 揽收快递（按序号）\n";
            cout << "3. 查询我的任务\n";
            cout << "0. 退出登录\n";
            int c = inputInt("请选择:");
            if (c == 1) show(net.request("COURIER_LIST_WAIT|" + username));
            else if (c == 2) pickupExpress();
            else if (c == 3) courierQuery();
            else if (c == 0) break;
            else cout << "无效选择。\n";
        }
    }

    void adminAssign() {
        string res = net.request("ADMIN_ASSIGN_LIST");
        showNumberedExpressList(res);
        int n = inputInt("请输入要分配的快递序号:");
        string id = getExpressIdByNumber(res, n);
        if (id.empty()) {
            cout << "序号无效。\n";
            return;
        }
        string courier = input("请输入快递员账号:");
        show(net.request("ADMIN_ASSIGN|" + id + "|" + courier));
    }
    void adminQuery() {
        cout << "\n===== 管理员查询快递 =====\n";
        string q = chooseQueryField(true, true);
        if (q == "all|") show(net.request("ADMIN_QUERY"));
        else show(net.request("ADMIN_QUERY_FIELD|" + q));
    }
    void adminCourierTasks() {
        string c = input("请输入快递员账号，输入ALL查看全部:");
        if (c.empty()) c = "ALL";
        show(net.request("ADMIN_COURIER_TASKS|" + c));
    }
    void adminMenu() {
        while (true) {
            cout << "1. 添加快递员\n";
            cout << "2. 删除快递员\n";
            cout << "3. 查看待分配快递和快递员\n";
            cout << "4. 分配快递员（按序号）\n";
            cout << "5. 查看所有用户\n";
            cout << "6. 查询全部快递\n";
            cout << "7. 查看所有快递员\n";
            cout << "8. 查看快递员任务\n";
            cout << "9. 查看管理员信息\n";
            cout << "10. 查看系统日志\n";
            cout << "11. 查看用户评价\n";
            cout << "0. 退出登录\n";
            int c = inputInt("请选择:");
            if (c == 1) {
                string u = input("快递员账号:");
                string n = input("姓名:");
                string ph = input("电话:");
                string pw = input("密码:");
                show(net.request("ADMIN_ADD_COURIER|" + u + "|" + n + "|" + ph + "|" + pw));
            }
            else if (c == 2) {
                string u = input("要删除的快递员账号:");
                show(net.request("ADMIN_DELETE_COURIER|" + u));
            }
            else if (c == 3) show(net.request("ADMIN_ASSIGN_LIST"));
            else if (c == 4) adminAssign();
            else if (c == 5) show(net.request("ADMIN_USERS"));
            else if (c == 6) adminQuery();
            else if (c == 7) show(net.request("ADMIN_COURIERS"));
            else if (c == 8) adminCourierTasks();
            else if (c == 9) show(net.request("ADMIN_INFO"));
            else if (c == 10) show(net.request("LOGS"));

            else if (c == 11)
            {
                show(net.request("VIEW_EVALUATIONS"));
            }

            else if (c == 0) break;
            else cout << "无效选择。\n";
        }
    }

public:
    void run() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        if (!net.init()) {
            cout << "Socket初始化失败\n";
            return;
        }
        string ip = input("服务器IP，直接回车默认127.0.0.1:");
        if (ip.empty()) ip = "127.0.0.1";
        if (!net.connectServer(ip, 8888)) {
            cout << "连接服务器失败，请先运行服务器。\n";
            net.cleanup();
            return;
        }
        string hello;
        net.readResponse(hello);
        cout << hello;
        while (true) {
            cout << "\n========== 物流管理系统客户端 ==========\n";
            cout << "1. 用户注册\n";
            cout << "2. 用户登录\n";
            cout << "3. 快递员登录\n";
            cout << "4. 管理员登录\n";
            cout << "0. 退出\n";
            int c = inputInt("请选择:");
            if (c == 1) registerUser();
            else if (c == 2) { if (login("USER")) userMenu(); }
            else if (c == 3) { if (login("COURIER")) courierMenu(); }
            else if (c == 4) { if (login("ADMIN")) adminMenu(); }
            else if (c == 0) { net.sendLine("QUIT"); break; }
            else cout << "无效选择。\n";
        }
        net.closeSocket();
        net.cleanup();
    }
};

int main() {
    ClientApp app;
    app.run();
    return 0;
}
