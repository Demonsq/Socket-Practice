#include "socket.h"
#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>

int NWINDOW;      // 滑动窗口大小

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 设置字体大小
    QFont font  = app.font();
    font.setPointSize(10);
    app.setFont(font);

    WSADATA wsaData;
    SOCKET toSrv;
    sockaddr_in srvAddr;

    // 初始化winsock
    WSAStartup(0x0101, &wsaData);

    // 作为客户端时的socket
    toSrv = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器信息（用于将本机连接到服务器）
    char ip[20];
    FILE *fp = fopen("ipconfig.txt", "r");
    if(fp == NULL){
        cout << "IP配置文件打开失败";
        return 0;
    }
    else{
        memset(ip, 0, 20);
        fscanf(fp, "%d", &NWINDOW);
        fscanf(fp, "%s", ip);
        fclose(fp);
    }
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(SRVPORT);                  // 连接到服务器5050端口
    srvAddr.sin_addr.S_un.S_addr = inet_addr(ip);       // 服务器ip地址

    LoginDialog dlg(toSrv, srvAddr);                    // 登录窗口
    if(dlg.exec() == QDialog::Accepted){
        MainWindow w(toSrv);                            // 主窗口
        w.show();
        app.exec();
    }

    return 0;
}
