#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <algorithm>
#include <QDialog>
#include <QMessageBox>
#include <QCloseEvent>

#define SRVPORT 5050            // 服务器监听端口
#define MAXCONN 30              // 同时最大连接数
#define BUFLEN 255              // 缓冲区长度
#define MAXLEN 1024             // udp数据包长度
#define TIMEOUT 100             // 超时时间

using namespace std;

extern int NWINDOW;             // 滑动窗口大小

// udp传输packet
struct DataGram{
    long seqnum;
    char data[MAXLEN];
};

#endif // SOCKET_H
