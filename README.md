# C++ 网络版物流管理系统

一个基于 C/S 架构的 C++ 物流管理系统课程项目，由单机版扩展为网络版。项目重点练习 TCP Socket 通信、自定义文本协议、多角色权限设计、物流订单状态流转和文件持久化。

## 项目特点

- 服务端监听 `8888` 端口，客户端通过 TCP Socket 与服务端通信。
- 使用自定义文本协议处理 `REGISTER_USER`、`LOGIN`、`SEND_EXPRESS`、`ADMIN_ASSIGN`、`COURIER_PICKUP`、`SIGN_BY_NUMBER`、`EVALUATE` 等请求。
- 支持用户、快递员、管理员 3 类角色，覆盖注册/登录、充值、寄件、分配、揽收、签收、撤回、评价、日志查询等 10+ 核心功能。
- 维护待揽收、待签收、已签收 3 种物流订单状态，支持未揽收撤回、按单号签收和快递员费用分成。
- 使用本地文本文件保存账号、订单、日志、评价等运行时数据，服务重启后可恢复业务状态。

## 目录结构

```text
.
├── client/          # Visual Studio 客户端工程
│   ├── client.sln
│   └── client/
│       ├── client.vcxproj
│       └── 源.cpp
└── server/          # Visual Studio 服务端工程
    ├── server.sln
    └── server/
        ├── server.vcxproj
        └── 源.cpp
```

## 运行方式

1. 使用 Visual Studio 打开 `server/server.sln`，编译并启动服务端。
2. 使用 Visual Studio 打开 `client/client.sln`，编译并启动客户端。
3. 先启动服务端，再启动一个或多个客户端进行注册、登录、寄件、揽收、签收等操作。

## GitHub 上传说明

本仓库不提交 `.exe`、`.obj`、`.pdb`、`.vs/`、`x64/Debug/` 等编译产物，也不提交 `users.txt`、`admin.txt`、`logs.txt` 等运行时数据文件。运行过程中产生的数据文件会由服务端自动读写。

该项目为课程/实验性质，账号密码以明文保存在本地文本文件中，不适用于生产环境。
