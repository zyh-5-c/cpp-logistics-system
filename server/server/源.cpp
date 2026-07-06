#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma execution_character_set("utf-8")

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <mutex>
#include <thread>
#include <algorithm>
#include <cctype>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

class User {
private:
    string username, name, phone, password, address;
    double balance;
public:
    User() : balance(0) {}
    User(const string& u, const string& n, const string& ph, const string& pw, double b, const string& ad)
        : username(u), name(n), phone(ph), password(pw), address(ad), balance(b) {
    }
    string getUsername() const { return username; }
    string getName() const { return name; }
    string getPhone() const { return phone; }
    string getAddress() const { return address; }
    double getBalance() const { return balance; }
    bool checkPassword(const string& pw) const { return password == pw; }
    void setPassword(const string& pw) { password = pw; }
    void recharge(double m) { if (m > 0) balance += m; }
    bool deduct(double m) { if (m <= 0 || balance < m) return false; balance -= m; return true; }
    string toString() const {
        ostringstream os;
        os << "用户名:" << username << " 姓名:" << name << " 电话:" << phone
            << " 余额:" << fixed << setprecision(2) << balance << " 地址:" << address;
        return os.str();
    }
    string serialize() const {
        ostringstream os;
        os << username << '|' << name << '|' << phone << '|' << password << '|'
            << fixed << setprecision(2) << balance << '|' << address;
        return os.str();
    }
    static User deserialize(const string& line) {
        stringstream ss(line);
        string u, n, ph, pw, bs, ad;
        getline(ss, u, '|'); getline(ss, n, '|'); getline(ss, ph, '|'); getline(ss, pw, '|'); getline(ss, bs, '|'); getline(ss, ad);
        double b = 0; try { b = stod(bs); }
        catch (...) {}
        return User(u, n, ph, pw, b, ad);
    }
};

class Admin {
private:
    string username, name, password;
    double balance;
public:
    Admin() : username("zyh"), name("物流公司管理员"), password("qwe182858"), balance(0) {}
    bool checkLogin(const string& u, const string& p) const { return username == u && password == p; }
    string getUsername() const { return username; }
    double getBalance() const { return balance; }
    void addBalance(double m) { if (m > 0) balance += m; }
    bool deduct(double m) { if (m <= 0 || balance < m) return false; balance -= m; return true; }
    string toString() const {
        ostringstream os;
        os << "管理员:" << username << " 姓名:" << name << " 公司余额:" << fixed << setprecision(2) << balance;
        return os.str();
    }
    string serialize() const {
        ostringstream os; os << username << '|' << name << '|' << password << '|' << fixed << setprecision(2) << balance; return os.str();
    }
    void deserialize(const string& line) {
        stringstream ss(line); string bs;
        getline(ss, username, '|'); getline(ss, name, '|'); getline(ss, password, '|'); getline(ss, bs);
        try { balance = stod(bs); }
        catch (...) { balance = 0; }
    }
};

class Courier {
private:
    string username, name, phone, password;
    double balance;
public:
    Courier() : balance(0) {}
    Courier(const string& u, const string& n, const string& ph, const string& pw, double b)
        : username(u), name(n), phone(ph), password(pw), balance(b) {
    }
    string getUsername() const { return username; }
    string getName() const { return name; }
    string getPhone() const { return phone; }
    double getBalance() const { return balance; }
    bool checkPassword(const string& pw) const { return password == pw; }
    void addBalance(double m) { if (m > 0) balance += m; }
    string toString() const {
        ostringstream os;
        os << "快递员账号:" << username << " 姓名:" << name << " 电话:" << phone
            << " 余额:" << fixed << setprecision(2) << balance;
        return os.str();
    }
    string serialize() const {
        ostringstream os; os << username << '|' << name << '|' << phone << '|' << password << '|' << fixed << setprecision(2) << balance; return os.str();
    }
    static Courier deserialize(const string& line) {
        stringstream ss(line); string u, n, ph, pw, bs;
        getline(ss, u, '|'); getline(ss, n, '|'); getline(ss, ph, '|'); getline(ss, pw, '|'); getline(ss, bs);
        double b = 0; try { b = stod(bs); }
        catch (...) {}
        return Courier(u, n, ph, pw, b);
    }
};

class Item {
protected:
    string desc;
public:
    Item(const string& d) : desc(d) {}
    virtual ~Item() {}
    virtual double getPrice() const = 0;
    virtual string getType() const = 0;
};
class FragileItem : public Item {
private: double weight;
public:
    FragileItem(const string& d, double w) : Item(d), weight(w) {}
    double getPrice() const override { return weight * 8.0; }
    string getType() const override { return "易碎品"; }
};
class BookItem : public Item {
private: int count;
public:
    BookItem(const string& d, int c) : Item(d), count(c) {}
    double getPrice() const override { return count * 2.0; }
    string getType() const override { return "图书"; }
};
class NormalItem : public Item {
private: double weight;
public:
    NormalItem(const string& d, double w) : Item(d), weight(w) {}
    double getPrice() const override { return weight * 5.0; }
    string getType() const override { return "普通快递"; }
};

class Express {
private:
    string id, sendTime, receiveTime, sender, receiver, status, description, itemType, courier;
    double fee;
public:
    Express() : fee(0) {}
    Express(const string& i, const string& st, const string& s, const string& r, const string& de, const string& type, double f)
        : id(i), sendTime(st), receiveTime(""), sender(s), receiver(r), status("待揽收"),
        description(de), itemType(type), courier(""), fee(f) {
    }
    string getId() const { return id; }
    string getSender() const { return sender; }
    string getReceiver() const { return receiver; }
    string getStatus() const { return status; }
    string getSendTime() const { return sendTime; }
    string getDescription() const { return description; }
    string getItemType() const { return itemType; }
    string getCourier() const { return courier; }
    double getFee() const { return fee; }
    bool relatedTo(const string& u) const { return sender == u || receiver == u; }
    bool isWaitingForCourier(const string& c) const { return courier == c && status == "待揽收"; }
    void assignCourier(const string& c) { courier = c; }
    void pickup() { status = "待签收"; }
    void sign(const string& t) { status = "已签收"; receiveTime = t; }
    bool matchField(const string& field, const string& key) const {
        if (field == "sender") return sender.find(key) != string::npos;
        if (field == "receiver") return receiver.find(key) != string::npos;
        if (field == "id") return id.find(key) != string::npos;
        if (field == "status") return status.find(key) != string::npos;
        if (field == "date") return sendTime.find(key) != string::npos;
        if (field == "desc") return description.find(key) != string::npos;
        if (field == "type") return itemType.find(key) != string::npos;
        if (field == "courier") return courier.find(key) != string::npos;
        return id.find(key) != string::npos || sender.find(key) != string::npos || receiver.find(key) != string::npos ||
            status.find(key) != string::npos || sendTime.find(key) != string::npos || description.find(key) != string::npos ||
            itemType.find(key) != string::npos || courier.find(key) != string::npos;
    }
    string toString() const {
        ostringstream os;
        os << "单号:" << id << "\n";
        os << "寄件人:" << sender << " 收件人:" << receiver << "\n";
        os << "类型:" << itemType << " 描述:" << description << " 费用:" << fixed << setprecision(2) << fee << "\n";
        os << "快递员:" << (courier.empty() ? "未分配" : courier) << " 状态:" << status << "\n";
        os << "寄送时间:" << sendTime << " 接收时间:" << (receiveTime.empty() ? "未签收" : receiveTime) << "\n";
        return os.str();
    }
    string serialize() const {
        ostringstream os;
        os << id << '|' << sendTime << '|' << receiveTime << '|' << sender << '|' << receiver << '|'
            << status << '|' << description << '|' << itemType << '|' << fixed << setprecision(2) << fee << '|' << courier;
        return os.str();
    }
    static Express deserialize(const string& line) {
        Express e; string fs; stringstream ss(line);
        getline(ss, e.id, '|'); getline(ss, e.sendTime, '|'); getline(ss, e.receiveTime, '|'); getline(ss, e.sender, '|');
        getline(ss, e.receiver, '|'); getline(ss, e.status, '|'); getline(ss, e.description, '|'); getline(ss, e.itemType, '|');
        getline(ss, fs, '|'); getline(ss, e.courier);
        try { e.fee = stod(fs); }
        catch (...) { e.fee = 0; }
        return e;
    }
};

class LogisticsServer {
private:
    vector<User> users;
    vector<Courier> couriers;
    vector<Express> expresses;
    Admin admin;
    mutex dataMutex;
    const string userFile = "users.txt";
    const string courierFile = "couriers.txt";
    const string expressFile = "expresses.txt";
    const string adminFile = "admin.txt";
    const string logFile = "logs.txt";
    const string evaluateFile = "evaluations.txt";
    SOCKET listenSock = INVALID_SOCKET;

    vector<string> split(const string& s) const {
        vector<string> v; string item; stringstream ss(s);
        while (getline(ss, item, '|')) v.push_back(item);
        return v;
    }
    string ok(const string& msg) const { return "OK\n" + msg + "\n"; }
    string fail(const string& msg) const { return "FAIL\n" + msg + "\n"; }
    string money(double x) const { ostringstream os; os << fixed << setprecision(2) << x; return os.str(); }
    bool validPhone(const string& ph) const {
        if (ph.size() < 6 || ph.size() > 20) return false;
        for (char c : ph) if (!isdigit((unsigned char)c)) return false;
        return true;
    }
    string nowTime() const {
        time_t t = time(nullptr); tm lt; localtime_s(&lt, &t);
        ostringstream os; os << put_time(&lt, "%Y-%m-%d_%H:%M:%S"); return os.str();
    }
    string newExpressId() const { return "EX" + to_string(time(nullptr)) + "_" + to_string(expresses.size() + 1); }
    void log(const string& s) const { ofstream out(logFile, ios::app); out << "[" << nowTime() << "] " << s << "\n"; }
    void saveEvaluation(const string& expressId, const string& courier, const string& username,
        const string& score, const string& content) const {
        ofstream out("evaluations.txt", ios::app);
        out << expressId << "|" << courier << "|" << username << "|" << score << "|" << content << "\n";
    }

    int findUser(const string& u) const { for (int i = 0; i < (int)users.size(); ++i) if (users[i].getUsername() == u) return i; return -1; }
    int findCourier(const string& u) const { for (int i = 0; i < (int)couriers.size(); ++i) if (couriers[i].getUsername() == u) return i; return -1; }
    int findExpress(const string& id) const { for (int i = 0; i < (int)expresses.size(); ++i) if (expresses[i].getId() == id) return i; return -1; }

    void loadUsers() { users.clear(); ifstream in(userFile); string line; while (getline(in, line)) if (!line.empty()) users.push_back(User::deserialize(line)); }
    void loadCouriers() { couriers.clear(); ifstream in(courierFile); string line; while (getline(in, line)) if (!line.empty()) couriers.push_back(Courier::deserialize(line)); }
    void loadExpresses() { expresses.clear(); ifstream in(expressFile); string line; while (getline(in, line)) if (!line.empty()) expresses.push_back(Express::deserialize(line)); }
    void loadAdmin() { ifstream in(adminFile); string line; if (getline(in, line) && !line.empty()) admin.deserialize(line); }
    void saveUsers() const { ofstream out(userFile); for (const User& u : users) out << u.serialize() << "\n"; }
    void saveCouriers() const { ofstream out(courierFile); for (const Courier& c : couriers) out << c.serialize() << "\n"; }
    void saveExpresses() const { ofstream out(expressFile); for (const Express& e : expresses) out << e.serialize() << "\n"; }
    void saveAdmin() const { ofstream out(adminFile); out << admin.serialize() << "\n"; }
    void saveAll() const { saveUsers(); saveCouriers(); saveExpresses(); saveAdmin(); }

    string expressList(const vector<int>& idxs) const {
        ostringstream os;
        for (int i : idxs) os << expresses[i].toString() << "---\n";
        os << "共" << idxs.size() << "件\n";
        return os.str();
    }

    bool sendResponse(SOCKET s, const string& msg) const {
        string data = msg + "END\n";
        const char* p = data.c_str();
        int left = (int)data.size();
        while (left > 0) {
            int n = send(s, p, left, 0);
            if (n <= 0) return false;
            p += n; left -= n;
        }
        return true;
    }
    bool recvLine(SOCKET s, string& line) const {
        line.clear(); char ch;
        while (true) {
            int n = recv(s, &ch, 1, 0);
            if (n <= 0) return false;
            if (ch == '\n') break;
            if (ch != '\r') line.push_back(ch);
        }
        return true;
    }
    // 功能：统一处理客户端请求
// 参数：req为客户端发送的命令字符串
// 返回值：处理结果字符串
// 说明：根据命令类型调用对应业务逻辑
    string handle(const string& req) {
        lock_guard<mutex> lock(dataMutex);
        vector<string> a = split(req);
        if (a.empty()) return fail("空请求");
        string cmd = a[0];

        if (cmd == "REGISTER_USER") {
            if (a.size() < 6) return fail("参数不足");
            if (a[1].empty() || a[4].empty() || a[5].empty()) return fail("用户名、密码、地址不能为空");
            if (a[1] == admin.getUsername() || findUser(a[1]) != -1 || findCourier(a[1]) != -1) return fail("账号已存在");
            if (!validPhone(a[3])) return fail("电话应为6到20位数字");
            users.push_back(User(a[1], a[2], a[3], a[4], 0, a[5]));
            saveAll(); log("用户注册:" + a[1]);
            return ok("注册成功");
        }
        if (cmd == "LOGIN") {
            if (a.size() < 4) return fail("参数不足");
            string role = a[1], u = a[2], p = a[3];
            if (role == "USER") { int i = findUser(u); if (i != -1 && users[i].checkPassword(p)) return ok("USER|" + u); }
            if (role == "COURIER") { int i = findCourier(u); if (i != -1 && couriers[i].checkPassword(p)) return ok("COURIER|" + u); }
            if (role == "ADMIN" && admin.checkLogin(u, p)) return ok("ADMIN|" + u);
            return fail("账号或密码错误");
        }
        if (cmd == "CHANGE_PASSWORD") {
            if (a.size() < 4) return fail("参数不足");
            int i = findUser(a[1]);
            if (i == -1) return fail("用户不存在");
            if (!users[i].checkPassword(a[2])) return fail("原密码错误");
            if (a[3].empty()) return fail("新密码不能为空");
            users[i].setPassword(a[3]); saveAll(); log("用户修改密码:" + a[1]);
            return ok("密码修改成功");
        }
        if (cmd == "BALANCE") {
            if (a.size() < 2) return fail("参数不足");
            int i = findUser(a[1]); if (i == -1) return fail("用户不存在");
            return ok("当前余额:" + money(users[i].getBalance()));
        }
        if (cmd == "RECHARGE") {
            if (a.size() < 3) return fail("参数不足");
            int i = findUser(a[1]); if (i == -1) return fail("用户不存在");
            double m = 0; try { m = stod(a[2]); }
            catch (...) { return fail("金额非法"); }
            if (m <= 0) return fail("金额必须大于0");
            users[i].recharge(m); saveAll(); log("用户充值:" + a[1]);
            return ok("充值成功，余额:" + money(users[i].getBalance()));
        }
        if (cmd == "SEND_EXPRESS") {
            if (a.size() < 6) return fail("参数不足");
            string sender = a[1], receiver = a[2], desc = a[3];
            int si = findUser(sender), ri = findUser(receiver);
            if (si == -1) return fail("寄件用户不存在");
            if (ri == -1) return fail("收件用户不存在");
            if (sender == receiver) return fail("不能给自己发送快递");
            if (desc.empty()) return fail("物品描述不能为空");
            int type = 0; double num = 0;
            try { type = stoi(a[4]); num = stod(a[5]); }
            catch (...) { return fail("类型或数量非法"); }
            if (num <= 0) return fail("数量必须大于0");
            string itemType; double fee = 0;
            if (type == 1) { FragileItem item(desc, num); itemType = item.getType(); fee = item.getPrice(); }
            else if (type == 2) { BookItem item(desc, (int)num); itemType = item.getType(); fee = item.getPrice(); }
            else if (type == 3) { NormalItem item(desc, num); itemType = item.getType(); fee = item.getPrice(); }
            else return fail("快递类型无效");
            if (!users[si].deduct(fee)) return fail("余额不足，本次费用:" + money(fee));
            admin.addBalance(fee);
            string id = newExpressId();
            expresses.push_back(Express(id, nowTime(), sender, receiver, desc, itemType, fee));
            saveAll(); log("发送快递:" + id);
            return ok("发送成功\n单号:" + id + "\n费用:" + money(fee) + "\n状态:待揽收");
        }
        if (cmd == "LIST_PENDING_SIGN") {
            if (a.size() < 2) return fail("参数不足");
            vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (expresses[i].getReceiver() == a[1] && expresses[i].getStatus() == "待签收") ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "SIGN") {
            if (a.size() < 3) return fail("参数不足");
            int i = findExpress(a[2]);
            if (i == -1) return fail("快递不存在");
            if (expresses[i].getReceiver() != a[1]) return fail("只能签收自己的快递");
            if (expresses[i].getStatus() != "待签收") return fail("当前状态不能签收");
            expresses[i].sign(nowTime()); saveAll(); log("签收快递:" + a[2]);
            return ok("签收成功");
        }
        if (cmd == "QUERY_MY" || cmd == "QUERY_MY_FIELD") {
            if (a.size() < 2) return fail("参数不足");
            string u = a[1], field = "all", key = "";
            if (cmd == "QUERY_MY_FIELD") {
                if (a.size() < 4) return fail("参数不足");
                field = a[2]; key = a[3];
            }
            vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (expresses[i].relatedTo(u) && (field == "all" || expresses[i].matchField(field, key))) ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "ADMIN_ADD_COURIER") {
            if (a.size() < 5) return fail("参数不足");
            if (a[1].empty() || a[4].empty()) return fail("账号和密码不能为空");
            if (a[1] == admin.getUsername() || findUser(a[1]) != -1 || findCourier(a[1]) != -1) return fail("账号已存在");
            if (!validPhone(a[3])) return fail("电话应为6到20位数字");
            couriers.push_back(Courier(a[1], a[2], a[3], a[4], 0));
            saveAll(); log("添加快递员:" + a[1]);
            return ok("快递员添加成功");
        }
        if (cmd == "ADMIN_DELETE_COURIER") {
            if (a.size() < 2) return fail("参数不足");
            int ci = findCourier(a[1]); if (ci == -1) return fail("快递员不存在");
            for (const Express& e : expresses)
                if (e.getCourier() == a[1] && e.getStatus() == "待揽收") return fail("该快递员还有待揽收任务，不能删除");
            couriers.erase(couriers.begin() + ci);
            saveAll(); log("删除快递员:" + a[1]);
            return ok("删除成功");
        }
        if (cmd == "ADMIN_ASSIGN_LIST") {
            ostringstream os;
            os << "未分配快递:\n";
            int cnt = 0;
            for (const Express& e : expresses)
                if (e.getStatus() == "待揽收" && e.getCourier().empty()) { os << e.toString() << "---\n"; cnt++; }
            os << "快递员:\n";
            for (const Courier& c : couriers) os << c.toString() << "\n";
            os << "共" << cnt << "件待分配\n";
            return ok(os.str());
        }
        if (cmd == "ADMIN_ASSIGN") {
            if (a.size() < 3) return fail("参数不足");
            int ei = findExpress(a[1]), ci = findCourier(a[2]);
            if (ei == -1) return fail("快递不存在");
            if (ci == -1) return fail("快递员不存在");
            if (expresses[ei].getStatus() != "待揽收") return fail("只有待揽收快递可分配");
            expresses[ei].assignCourier(a[2]); saveAll(); log("分配快递:" + a[1] + "给" + a[2]);
            return ok("分配成功");
        }
        if (cmd == "COURIER_LIST_WAIT") {
            if (a.size() < 2) return fail("参数不足");
            vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (expresses[i].isWaitingForCourier(a[1])) ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "COURIER_PICKUP") {
            if (a.size() < 3) return fail("参数不足");
            int ci = findCourier(a[1]), ei = findExpress(a[2]);
            if (ci == -1) return fail("快递员不存在");
            if (ei == -1) return fail("快递不存在");
            if (!expresses[ei].isWaitingForCourier(a[1])) return fail("该快递不属于你或状态不是待揽收");
            double reward = expresses[ei].getFee() * 0.5;
            if (!admin.deduct(reward)) return fail("公司账户余额不足");
            couriers[ci].addBalance(reward);
            expresses[ei].pickup();
            saveAll(); log("快递员揽收:" + a[2]);
            return ok("揽收成功，获得:" + money(reward));
        }
        if (cmd == "COURIER_QUERY" || cmd == "COURIER_QUERY_FIELD") {
            if (a.size() < 2) return fail("参数不足");
            string c = a[1], field = "all", key = "";
            if (cmd == "COURIER_QUERY_FIELD") {
                if (a.size() < 4) return fail("参数不足");
                field = a[2]; key = a[3];
            }
            vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (expresses[i].getCourier() == c && (field == "all" || expresses[i].matchField(field, key))) ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "ADMIN_USERS") {
            ostringstream os; for (const User& u : users) os << u.toString() << "\n"; os << "共" << users.size() << "个用户\n"; return ok(os.str());
        }
        if (cmd == "ADMIN_COURIERS") {
            ostringstream os; for (const Courier& c : couriers) os << c.toString() << "\n"; os << "共" << couriers.size() << "个快递员\n"; return ok(os.str());
        }
        if (cmd == "ADMIN_QUERY" || cmd == "ADMIN_QUERY_FIELD") {
            string field = "all", key = "";
            if (cmd == "ADMIN_QUERY_FIELD") {
                if (a.size() < 3) return fail("参数不足");
                field = a[1]; key = a[2];
            }
            vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (field == "all" || expresses[i].matchField(field, key)) ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "ADMIN_COURIER_TASKS") {
            if (a.size() < 2) return fail("参数不足");
            string c = a[1]; vector<int> ids;
            for (int i = 0; i < (int)expresses.size(); ++i)
                if (c == "ALL" || expresses[i].getCourier() == c) ids.push_back(i);
            return ok(expressList(ids));
        }
        if (cmd == "ADMIN_INFO") return ok(admin.toString());
        if (cmd == "LOGS") {
            ifstream in(logFile); string line; ostringstream os;
            while (getline(in, line)) os << line << "\n";
            return ok(os.str().empty() ? "暂无日志\n" : os.str());
        }
        
        if (cmd == "EVALUATE") {
            if (a.size() < 5) return fail("参数不足");

            string username = a[1];
            string expressId = a[2];
            string score = a[3];
            string content = a[4];

            int scoreNum;
            try {
                scoreNum = stoi(score);
            }
            catch (...) {
                return fail("评分必须是数字");
            }

            if (scoreNum < 1 || scoreNum > 5) {
                return fail("评分必须在1到5之间");
            }

            int idx = findExpress(expressId);

            if (idx == -1) {
                return fail("快递不存在");
            }

            if (expresses[idx].getReceiver() != username) {
                return fail("只能评价自己收到的快递");
            }

            if (expresses[idx].getStatus() != "已签收") {
                return fail("只能评价已签收的快递");
            }

            string courier = expresses[idx].getCourier();

            saveEvaluation(
                expressId,
                courier,
                username,
                score,
                content
            );

            log("用户评价快递:" + username +
                ", 单号:" + expressId +
                ", 快递员:" + courier);

            return ok("评价提交成功");
        }

        if (cmd == "VIEW_EVALUATIONS") {
            ifstream in("evaluations.txt");

            if (!in) {
                return ok("暂无评价记录");
            }

            string line;
            ostringstream oss;
            int count = 0;

            while (getline(in, line)) {
                if (line.empty()) continue;

                stringstream ss(line);
                string expressId, courier, username, score, content;

                getline(ss, expressId, '|');
                getline(ss, courier, '|');
                getline(ss, username, '|');
                getline(ss, score, '|');
                getline(ss, content);

                oss << "快递单号:" << expressId << "\n";
                oss << "快递员:" << courier << "\n";
                oss << "评价用户:" << username << "\n";
                oss << "评分:" << score << "星\n";
                oss << "评价内容:" << content << "\n";
                oss << "-------------------------\n";

                count++;
            }

            oss << "共" << count << " 条评价记录\n";

            return ok(oss.str());
        }
        
        return fail("未知命令:" + cmd);
    }
    // 功能：处理单个客户端连接
// 参数：clientSock为客户端Socket
// 说明：每个客户端对应一个独立线程
    void clientThread(SOCKET client) {
        sendResponse(client, "物流管理系统服务器连接成功\n");
        string line;
        while (recvLine(client, line)) {
            if (line == "QUIT") break;
            sendResponse(client, handle(line));
        }
        closesocket(client);
        cout << "客户端断开连接\n";
    }

public:
    void loadAll() {
        lock_guard<mutex> lock(dataMutex);
        loadUsers(); loadCouriers(); loadExpresses(); loadAdmin();
    }
  // 功能：启动物流管理系统服务器
// 参数：port为监听端口
// 流程：socket 创建监听窗口-> bind 绑定ip和端口-> listen 等待客户-> accept接受
// 说明：负责等待客户端连接
    void run(int port) {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cout << "WSAStartup失败\n"; return; }
        listenSock = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSock == INVALID_SOCKET) { cout << "创建socket失败\n"; return; }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) { cout << "绑定端口失败\n"; closesocket(listenSock); WSACleanup(); return; }
        if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) { cout << "监听失败\n"; closesocket(listenSock); WSACleanup(); return; }
        cout << "服务器已启动，端口:" << port << "\n";
        cout << "等待客户端连接...\n";
        while (true) {
            SOCKET client = accept(listenSock, nullptr, nullptr);
            if (client == INVALID_SOCKET) continue;
            cout << "客户端已连接\n";
            thread(&LogisticsServer::clientThread, this, client).detach();
        }
    }
};

int main() {
    LogisticsServer server;
    server.loadAll();
    server.run(8888);
    return 0;
}
