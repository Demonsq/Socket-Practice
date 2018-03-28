#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableWidget->verticalHeader()->setHidden(true);                     //隐藏行表头
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);    //禁止修改
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    // 信号绑定
    qRegisterMetaType<int>("int &");
    connect(this, SIGNAL(log_signal(const char*)), this, SLOT(update_log(const char*)));
    connect(this, SIGNAL(usrList_signal(int&,int&)), this, SLOT(update_usrList(int&,int&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(addUsrToList(const char*)), this, SLOT(addTolist(const char*)));

    // 从文件中读取用户信息
    UserInfo temp;
    int num;
    FILE *fp;
    if ((fp = fopen("user_Information.db", "rb")) == NULL)
        ui->textBrowser->append("用户信息加载失败.\n");
    else {
        ui->textBrowser->append("开始加载用户信息.\n");
        fseek(fp, 0, SEEK_END);
        num = ftell(fp) / (sizeof(int)+20+20+20+sizeof(sockaddr_in));
        rewind(fp);
        char buff[20];
        for (int i = 0; i != num; ++i) {
            fread(&temp.flag, sizeof(int), 1, fp);  // flag
            temp.flag = 0;
            memset(buff, 0, 20);
            fread(buff, 20, 1, fp);                 // user_name
            for(int j = 0; j != 20; ++j)
                buff[j] ^= 1;
            temp.user_name = buff;
            memset(buff, 0, 20);
            fread(buff, 20, 1, fp);                 // passward
            for(int j = 0; j != 20; ++j)
                buff[j] ^= 1;
            temp.passward = buff;
            memset(buff, 0, 20);
            fread(buff, 20, 1, fp);                 // tel
            for(int j = 0; j != 20; ++j)
                buff[j] ^= 1;
            temp.tel = buff;
            fread(&temp.userListenAddr, sizeof(sockaddr_in), 1, fp);
            usr.push_back(temp);

            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(temp.user_name.c_str()));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem("离线"));
            ui->tableWidget->item(i,0)->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignCenter);
        }
        fclose(fp);
    }

    // 开始监听
    srv.begin_listen();
    ui->textBrowser->append("开始监听.\n");

    // 处理连接请求
    std::thread deal_acc(&MainWindow::confirm_thread, this, ref(srv), ref(usr));
    deal_acc.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
    closesocket(srv.srvSock);
    WSACleanup();
}

// 检测用户登录、注册或找回密码(子线程1
void MainWindow::confirm_thread(SrvInfo &srv, vector<UserInfo> &usr)
{
    string temp;
    char name[BUFLEN];
    char pwd[BUFLEN];
    char tel[BUFLEN];
    sockaddr_in clientAddr;

    while (1) {
        SOCKET connSock;

        // 接收来自客户的连接请求
        connSock = accept(srv.srvSock, (LPSOCKADDR)&clientAddr, &srv.nAddrLen);
        if (connSock == INVALID_SOCKET) {
            log = "接收连接出错.\n";
            emit log_signal(log.c_str());
            return;
        }

        // 判断用户操作（1/2/3）
        if (srv.recvMessage(connSock, srv.recvBuf) == SOCKET_ERROR) continue;
        if (srv.recvMessage(connSock, name) == SOCKET_ERROR) continue;
        if (srv.recvMessage(connSock, pwd) == SOCKET_ERROR) continue;

        // 登录
        if (strcmp(srv.recvBuf, "1") == 0) {
            // 此时name保存的是用户名，pwd保存的是密码
            int id = -1;
            int login_flag = 0;
            for (UserInfo &i : usr) {
                ++id;
                if (strcmp(i.user_name.c_str(), name) == 0) {
                    // 用户名匹配成功
                    if (strcmp(i.passward.c_str(), pwd) == 0) {
                        // 密码匹配成功
                        login_flag = 1;
                        strcpy(srv.sendBuf, "1");
                        srv.sendMessage(connSock, srv.sendBuf);
                        // 登陆成功，更新用户状态信息
                        i.flag = 1;
                        emit usrList_signal(id, i.flag);
                        log = "用户"+i.user_name+"上线.\n";
                        emit log_signal(log.c_str());
                        break;
                    }
                    // 密码错误
                    else {
                        srv.sendMessage(connSock, "密码错误");
                        closesocket(connSock);
                        break;
                    }
                }
                else continue;
            }
            if (login_flag == 0) {
                // 用户名不存在
                strcpy(srv.sendBuf, "用户名不存在");
                srv.sendMessage(connSock, srv.sendBuf);
                closesocket(connSock);
            }
            // 创建用户子线程
            else {
                std::thread usr_thread(&MainWindow::online_user, this, id, connSock, ref(srv), ref(usr));
                usr_thread.detach();
            }
        }
        // 注册
        else if (strcmp(srv.recvBuf, "2") == 0) {
            srv.recvMessage(connSock, tel);
            // 此时name保存的是用户名，pwd保存的是密码, tel保存的是手机号
            int signup_flag = 0;
            for (const auto i : usr) {
                // 查看用户名是否存在
                if (strcmp(i.user_name.c_str(), name) == 0) {
                    signup_flag = 1;
                    strcpy(srv.sendBuf, "用户名已存在");
                    srv.sendMessage(connSock, srv.sendBuf);
                    closesocket(connSock);
                    break;
                }
                else {
                    // 检查手机号是否存在
                    if (strcmp(i.tel.c_str(), tel) == 0) {
                        signup_flag = 1;
                        strcpy(srv.sendBuf, "手机号已存在");
                        srv.sendMessage(connSock, srv.sendBuf);
                        closesocket(connSock);
                        break;
                    }
                    else continue;
                }
            }
            if (signup_flag == 0) {
                // 发送注册成功消息
                strcpy(srv.sendBuf, "2");
                srv.sendMessage(connSock, srv.sendBuf);
                closesocket(connSock);
                // 构造新用户信息并添加到数组
                UserInfo new_user = { 0, name, pwd, tel};
                emit addUsrToList(new_user.user_name.c_str());
                log = "用户"+new_user.user_name+"注册成功.\n";
                emit log_signal(log.c_str());
                usr.push_back(new_user);

                // 将用户信息写入文件
                char buff[20];
                FILE *fp = fopen("user_Information.db", "ab+");
                fwrite(&new_user.flag, sizeof(int), 1, fp);     // flag
                memset(buff, 0, 20);
                strcpy(buff, new_user.user_name.c_str());
                for(int j = 0; j != 20; ++j)
                    buff[j] ^= 1;
                fwrite(buff, 20, 1, fp);                        // user_name
                memset(buff, 0, 20);
                strcpy(buff, new_user.passward.c_str());
                for(int j = 0; j != 20; ++j)
                    buff[j] ^= 1;
                fwrite(buff, 20, 1, fp);                        // passward
                memset(buff, 0, 20);
                strcpy(buff, new_user.tel.c_str());
                for(int j = 0; j != 20; ++j)
                    buff[j] ^= 1;
                fwrite(buff, 20, 1, fp);                        // tel
                fwrite(&new_user.userListenAddr, sizeof(sockaddr_in), 1, fp);   // userListenAddr
                fclose(fp);
            }
        }
        // 找回密码
        else if (strcmp(srv.recvBuf, "3") == 0) {
            // 此时name保存的是用户名，tel保存的是手机号
            strcpy(tel, pwd);
            int find_flag = 0;
            for (const auto i : usr) {
                if (strcmp(i.user_name.c_str(), name) == 0) {
                    // 用户名匹配成功
                    cout << "tel:" << i.tel.c_str() << endl;
                    if (strcmp(i.tel.c_str(), tel) == 0) {
                        // 手机号匹配成功，将密码发回客户端
                        find_flag = 1;
                        strcpy(srv.sendBuf, "3");
                        srv.sendMessage(connSock, srv.sendBuf);
                        temp = "passward:" + i.passward;
                        strcpy(srv.sendBuf, temp.c_str());
                        srv.sendMessage(connSock, srv.sendBuf);
                        log = "用户"+i.user_name+"找回密码成功.\n";
                        emit log_signal(log.c_str());
                        closesocket(connSock);
                        break;
                    }
                    else {
                        // 提示客户端手机号错误
                        strcpy(srv.sendBuf, "手机号错误");
                        srv.sendMessage(connSock, srv.sendBuf);
                        closesocket(connSock);
                    }
                }
                else continue;
            }
            if (find_flag == 0) {
                // 用户名不存在
                strcpy(srv.sendBuf, "用户名不存在");
                srv.sendMessage(connSock, srv.sendBuf);
                closesocket(connSock);
            }
        }
        // 收到退出消息
        else break;
    }
}

// 与在线用户的通信(子线程2
void MainWindow::online_user(int id, SOCKET connSock, SrvInfo &srv, vector<UserInfo> &usr)
{
    FILE *fp;
    int find_flag = 0;
    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];

    // 向用户回传用户名
    if (srv.sendMessage(connSock, usr[id].user_name.c_str()) == SOCKET_ERROR) return;
    // 接收用户的TCP监听端口
    if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;
    memcpy(&(usr[id].userListenAddr), recvBuf, sizeof(sockaddr_in));

    // 发送用户列表
    itoa(usr.size(), sendBuf, 10);
    srv.sendMessage(connSock, sendBuf);
    for(UserInfo &i: usr)
        srv.sendMessage(connSock, i.user_name.c_str());

    // 判断用户是否存在离线消息
    if ((fp = fopen(usr[id].user_name.c_str(), "r")) != 0) {
        while (fread(sendBuf, sizeof(char), BUFLEN, fp) != 0)
            srv.sendMessage(connSock, sendBuf);
        srv.sendMessage(connSock, "end");
        fclose(fp);
        remove(usr[id].user_name.c_str());
    }
    else srv.sendMessage(connSock, "no message");

    while (1)
    {
        // 判断用户操作
        if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;
        // "1"表示查询用户名操作（用于TCP聊天）
        if (strcmp(recvBuf, "1") == 0) {
            // 接收用户发送的请求用户名
            if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;

            // 接收到客户机下线通知
            if (strcmp(recvBuf, "exit") == 0) {
                closesocket(connSock);
                usr[id].flag = 0;
                emit usrList_signal(id, usr[id].flag);
                log = "用户"+usr[id].user_name+"下线.\n";
                emit log_signal(log.c_str());
                return;
            }

            // 接收到用户查询请求
            find_flag = 0;
            for (auto i : usr)
                if (strcmp(i.user_name.c_str(), recvBuf) == 0) {
                    find_flag = 1;
                    break;
                }
                else continue;
            if (srv.sendMessage(connSock, "0" + find_flag) == SOCKET_ERROR)
                return;
        }
        // "2"表示用户请求给定用户的TCP监听端口
        else if (strcmp(recvBuf, "2") == 0) {
            if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;
            for (auto i : usr)
                if (strcmp(i.user_name.c_str(), recvBuf) == 0) {
                    memcpy(sendBuf, &(i.userListenAddr), sizeof(sockaddr_in));
                    if (srv.sendMessage(connSock, sendBuf) == SOCKET_ERROR) return;
                    break;
                }
                else continue;
        }
        // "3"表示服务器接收离线消息
        else if (strcmp(recvBuf, "3") == 0) {
            if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;         // 接收方用户名
            if ((fp = fopen(recvBuf, "a+")) != 0) {
                if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;		// 姓名、时间戳信息、正文
                fwrite(recvBuf, sizeof(char), BUFLEN, fp);
                fclose(fp);
            }
        }
        // "4"表示查询用户是否存在或是否在线
        // 在线则发送该用户tcp监听端口(用于udp文件传输)
        else {
            if (srv.recvMessage(connSock, recvBuf) == SOCKET_ERROR) return;
            find_flag = 0;
            for (auto i : usr) {
                if ((strcmp(i.user_name.c_str(), recvBuf) == 0) && (i.flag == 1) ) {
                    memcpy(sendBuf, &(i.userListenAddr), sizeof(sockaddr_in));
                    find_flag = 1;
                    break;
                }
                else continue;
            }
            if (find_flag == 1)
                srv.sendMessage(connSock, sendBuf);
            else
                srv.sendMessage(connSock, "0");
        }
    }
}

// 更新日志信息
void MainWindow::update_log(const char *log)
{
    ui->textBrowser->append(log);
}

// 更新用户信息
void MainWindow::update_usrList(int &id, int &flag)
{
    ui->tableWidget->setItem(id, 1, new QTableWidgetItem(flag==1?"在线":"离线"));
    ui->tableWidget->item(id, 1)->setTextAlignment(Qt::AlignCenter);
}

// 添加新的用户
void MainWindow::addTolist(const char *user_name)
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    ui->tableWidget->setItem(usr.size()-1, 0, new QTableWidgetItem(user_name));
    ui->tableWidget->setItem(usr.size()-1, 1, new QTableWidgetItem("离线"));
    ui->tableWidget->item(usr.size()-1, 0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->item(usr.size()-1, 1)->setTextAlignment(Qt::AlignCenter);
}
