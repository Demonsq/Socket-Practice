#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <algorithm>

#define MAXCONN 30			// 同时最大连接数
#define BUFLEN 255			// 缓冲区长度
#define PORT 5050			// 服务器监听端口

using namespace std;

struct UserInfo {
    int flag;
    string user_name;
    string passward;
    string tel;
    sockaddr_in userListenAddr;
};

struct SrvInfo {
    int nRC;
    int nAddrLen;
    SOCKET srvSock;
    WSADATA wsaData;
    sockaddr_in srvAddr;

    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];

    SrvInfo();
    ~SrvInfo() {
        closesocket(srvSock);
        WSACleanup();
    }

    void begin_listen();
    int recvMessage(SOCKET &p, char *buf);
    int sendMessage(SOCKET &p, const char *buf);
};

#endif // SERVER_H
