#include "server.h"

SrvInfo::SrvInfo() : nAddrLen(sizeof(sockaddr))
{
    //初始化 winsock
    nRC = WSAStartup(0x0101, &wsaData);
    if (nRC)
        cout << "Server initialize winsock error!" << endl;
    if (wsaData.wVersion != 0x0101)
    {
        cout << "Server's winsock version error!" << endl;
        WSACleanup();
    }
    cout << "Server's winsock initialized !" << endl;

    //创建 TCP socket
    srvSock = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSock == INVALID_SOCKET)
    {
        cout << "Server create socket error!" << endl;
        //        WSACleanup();
    }
    cout << "Server TCP socket create OK!" << endl;

    //绑定 socket to Server's IP and port 5050
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(PORT);
    srvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    nRC = ::bind(srvSock, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));

    if (nRC == SOCKET_ERROR)
    {
        cout << "Server socket bind error!" << endl;
        //        closesocket(srvSock);
        //        WSACleanup();
    }
    cout << "Server socket bind OK!" << endl;
}

void SrvInfo::begin_listen()
{
    //开始监听过程，等待客户的连接
    nRC = listen(srvSock, MAXCONN);
    if (nRC == SOCKET_ERROR)
        cout << "Server socket listen error!" << endl;
}

// 接收消息
int SrvInfo::recvMessage(SOCKET &p, char *buf) {
    memset(buf, '\0', BUFLEN);
    nRC = recv(p, buf, BUFLEN, 0);
    return nRC;
}

// 发送消息
int SrvInfo::sendMessage(SOCKET &p, const char *buf) {
    nRC = send(p, buf, BUFLEN, 0);
    return nRC;
}
