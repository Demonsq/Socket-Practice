#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(SOCKET &sock, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    toSrv(sock)
{
    ui->setupUi(this);
    ui->usrlineEdit->setFocus();

    // 获取当前用户用户名
    recv(toSrv,recvBuf,BUFLEN,0);
    name = recvBuf;
    setWindowTitle(name.c_str());   // 将标题设为用户名

    // 设置客户机信息（用于其他用户直接和本机连接）
    clientAddr.sin_family = AF_INET;		// 地址簇
    clientAddr.sin_port = htons(0);			// 系统自动分配端口号（1024-5000）
    clientAddr.sin_addr.S_un.S_addr = INADDR_ANY;       // 本机ip

    // 绑定TCP端口
    asSrv = socket(AF_INET, SOCK_STREAM, 0);
    ::bind(asSrv, (LPSOCKADDR)&clientAddr, sizeof(clientAddr));
    nRC = sizeof(sockaddr_in);
    sockaddr_in temp;
    getsockname(toSrv, (LPSOCKADDR)&temp, &nRC);
    getsockname(asSrv, (LPSOCKADDR)&clientAddr, &nRC);
    clientAddr.sin_addr = temp.sin_addr;

    cout << inet_ntoa(temp.sin_addr) << endl;
    cout << inet_ntoa(clientAddr.sin_addr) << endl;

    // 将TCP监听地址发给服务器保存
    memcpy(sendBuf, &clientAddr, sizeof(sockaddr_in));
    send(toSrv, sendBuf, BUFLEN, 0);
    // 开始监听
    listen(asSrv, MAXCONN);

    // 接收用户列表
    recv(toSrv, recvBuf, BUFLEN, 0);
    int i = atoi(recvBuf);
    while(i != 0){
        recv(toSrv, recvBuf, BUFLEN, 0);
        ui->userList->append(recvBuf);
        --i;
    }

    // 接收离线消息
    int flag = 0;
    char *s = 0;
    string user_name;
    recv(toSrv, recvBuf, BUFLEN, 0);
    while(strcmp(recvBuf, "end") != 0){
        if(strcmp(recvBuf, "no message") == 0)
            break;
        // 处理收到的离线消息
        s = strchr(recvBuf, ' ');
        *s = '\0';
        user_name = recvBuf;
        *s = ' ';
        // 根据用户名判断窗口是否已存在，存在则输出消息到该窗口
        flag = 0;
        for(ChatWindow *i : chat){
            if(i->user_name == user_name){
                flag = 1;
                i->show();
                i->displayMessage(recvBuf);
                i->displayMessage("\n");
                break;
            }
            else continue;
        }
        // 否则创建新的聊天窗口，以user_name作为标题
        if(flag == 0){
            // 创建聊天窗
            create_new_window(toSrv, name.c_str(), user_name.c_str(), recvBuf);
        }
        // 接收下一条
        recv(toSrv, recvBuf, BUFLEN, 0);
    }

    // 接收TCP连接请求
    std::thread paccept(&MainWindow::usrAccept, this);
    paccept.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 用于创建聊天窗口的槽函数
void MainWindow::create_new_window(SOCKET &sock, const char *name0,
                                   const char *name1, const char *buffer)
{
    newWindow = new ChatWindow(sock, name0, name1, this);
    chat.push_back(newWindow);
    newWindow->show();
    newWindow->displayMessage(buffer);
    newWindow->displayMessage("\n");
}

// 创建接收文件的槽函数
void MainWindow::create_getfile_window(SOCKET &tcp)
{
    RecvFile *getfile = new RecvFile(tcp, this);
    getfile->show();
}

// 接收用户TCP连接
void MainWindow::usrAccept()
{
    char *s;
    int flag;
    SOCKET connSock;
    string user_name;
    char buffer[BUFLEN];
    sockaddr_in newUserAddr;

    while(1){
        // 接收另一个用户的消息
        flag = 0;
        nRC = sizeof(sockaddr_in);
        connSock = accept(asSrv, (LPSOCKADDR)&newUserAddr, &nRC);
        if (connSock == INVALID_SOCKET) {
            cout << "Server accept connection request error!" << endl;
            return;
        }
        recv(connSock, buffer, BUFLEN, 0);
        // 若收到的是聊天消息（1表示聊天消息，2表示请求udp监听端口）
        if(strcmp(buffer, "1") == 0){
            recv(connSock, buffer, BUFLEN, 0);
            // 处理收到的报文
            s = strchr(buffer, ' ');
            *s = '\0';
            user_name = buffer;
            *s = ' ';
            // 根据用户名判断窗口是否已存在，存在则输出消息到该窗口
            for(ChatWindow *i : chat){
                if(i->user_name == user_name){
                    flag = 1;
                    i->show();
                    i->displayMessage(buffer);
                    i->displayMessage("\n");
                    closesocket(connSock);
                    break;
                }
                else continue;
            }
            // 否则创建新的聊天窗口，以user_name作为标题
            if(flag == 0){
                // 创建聊天窗
                qRegisterMetaType<SOCKET>("SOCKET&");
                connect(this, SIGNAL(request_new_window(SOCKET&,const char*,const char*,const char*)),
                                     this, SLOT(create_new_window(SOCKET&,const char*,const char*,const char*)),Qt::BlockingQueuedConnection);
                emit request_new_window(toSrv, name.c_str(), user_name.c_str(), buffer);
                closesocket(connSock);
            }
            else continue;
        }
        else{
            qRegisterMetaType<SOCKET>("SOCKET&");
            connect(this, SIGNAL(request_getfile_window(SOCKET&)),
                    this, SLOT(create_getfile_window(SOCKET&)), Qt::BlockingQueuedConnection);
            emit request_getfile_window(connSock);
        }
    }
}

// 下线（关闭窗口
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button=QMessageBox::question(this,tr("退出程序"),QString(tr("确认退出程序?")),QMessageBox::No|QMessageBox::Yes);
    if(button==QMessageBox::No)
    {
        event->ignore(); // 忽略退出信号，程序继续进行
    }
    else if(button==QMessageBox::Yes)
    {
        send(toSrv, "1", BUFLEN, 0);
        send(toSrv, "exit", BUFLEN, 0);
        closesocket(toSrv);
        for(auto i:chat)
            delete i;
        event->accept(); // 接受退出信号，程序退出
    }
}

// 向用户发起TCP连接
void MainWindow::on_chatButton_clicked()
{
    int flag = 0;
    string user_name;
    user_name = ui->usrlineEdit->text().toStdString();
    if(user_name == name){
        QMessageBox::information(this,tr("提示"),tr("无法与自己通信！"));
        ui->usrlineEdit->clear();
        return;
    }
    send(toSrv, "1", BUFLEN, 0);           // “1”表示执行请求用户名的操作
    if(send(toSrv, user_name.c_str(), BUFLEN, 0) != SOCKET_ERROR)
        ui->usrlineEdit->clear();

    if(recv(toSrv, recvBuf, BUFLEN, 0) == SOCKET_ERROR){
        QMessageBox::information(this,tr("提示"),tr("网络连接失败！"));
        return;
    }
    // 请求的用户名不存在
    if(strcmp(recvBuf, "0") == 0)
        QMessageBox::information(this,tr("提示"),tr("用户名不存在！"));
    // 用户名存在
    else{
        for(ChatWindow *i : chat){
            if(i->user_name == user_name){
                flag = 1;
                i->show();
            }
        }
        // 如果窗口不存在则创建新的窗口，以user_name为标题
        if(flag == 0){
            ChatWindow *newWindow = new ChatWindow(toSrv, name.c_str(), user_name.c_str(), this);
            newWindow->show();
            chat.push_back(newWindow);
        }
    }
}

// 点击传文件后的操作
void MainWindow::on_fileButton_clicked()
{
    string user_name;
    sockaddr_in usrAddr;
    char buffer[BUFLEN];

    user_name = ui->usrlineEdit->text().toStdString();
    if(user_name == name){
        QMessageBox::information(this,tr("提示"),tr("无法与自己通信！"));
        ui->usrlineEdit->clear();
        return;
    }

    // 发送用户名
    send(toSrv, "4", BUFLEN, 0);
    send(toSrv, ui->usrlineEdit->text().toStdString().c_str(), BUFLEN, 0);
    // 接收服务器返回值
    recv(toSrv, buffer, BUFLEN, 0);
    // 用户不存在或离线
    if(strcmp(buffer, "0") == 0)
        QMessageBox::information(this,tr("提示"),tr("用户不存在或处于离线状态！"));
    else{
        memcpy(&usrAddr, buffer, sizeof(sockaddr_in));
        SendFile *filetrans = new SendFile(usrAddr, name, user_name, this);
        filetrans->show();
    }
    ui->usrlineEdit->clear();
}
