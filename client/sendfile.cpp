#include "sendfile.h"
#include "ui_sendfile.h"

SendFile::SendFile(sockaddr_in &addr, string &myname, string &anoUsr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendFile),
    clientAddr(addr),
    name(myname)
{
    ui->setupUi(this);
    ui->sendButton->setEnabled(false);
    setWindowTitle(("To： " + anoUsr).c_str());

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),this, SLOT(resend()));

    nAddr = sizeof(sockaddr_in);
    // 设置客户机信息（用于其他用户直接和本机连接）
    clientUdp.sin_family = AF_INET;		// 地址簇
    clientUdp.sin_port = htons(0);			// 系统自动分配端口号（1024-5000）
    clientUdp.sin_addr.S_un.S_addr = INADDR_ANY;   // 本机ip
}

SendFile::~SendFile()
{
    delete ui;
}

// 关闭
void SendFile::closeEvent(QCloseEvent *event)
{
    closesocket(udpSock);
    delete timer;
    event->accept();
}

// 选择文件
void SendFile::on_chooseButton_clicked()
{
    filename = QFileDialog::getOpenFileName(
                this,
                "选择文件",
                "C:\\",
                0);
    if(filename.length() == 0){
        QMessageBox::information(this,tr("提示"),tr("请选择需要传输的文件"));
        return;
    }
    fp = fopen(filename.toStdString().c_str(), "r");
    if(fp == 0){
        QMessageBox::information(this,tr("提示"),tr("文件打开失败"));
        return;
    }
    else{
        fclose(fp);
        ui->filenameLine->setText(filename);
        QFileInfo file(filename);
        filelen = file.size();
        maxseq = filelen/MAXLEN+1;
        QString size;
        size.setNum(filelen/1024);
        ui->fileSize->setText(size+"KB");
        ui->sendButton->setEnabled(true);
    }
}

// 传输文件
void SendFile::on_sendButton_clicked()
{
    char buffer[BUFLEN];

    ui->sendButton->setEnabled(false);
    ui->chooseButton->setEnabled(false);

    SOCKET toUsr = socket(AF_INET, SOCK_STREAM, 0);
    if(::connect(toUsr, (LPSOCKADDR)&clientAddr, sizeof(sockaddr_in)) != SOCKET_ERROR){
        send(toUsr, "2", BUFLEN, 0);        // "2"表示发送给用户的是udp监听端口请求
        // 检测用户发回的回应信息
        QString fname;
        QFileInfo file(filename);
        fname = file.fileName();
        send(toUsr,name.c_str(), BUFLEN, 0);                    // 用户名
        send(toUsr, fname.toStdString().c_str(), BUFLEN, 0);    // 文件名
        send(toUsr, (char *)&filelen, sizeof(long), 0);         // 文件大小
        recv(toUsr, buffer, BUFLEN, 0);
        if(strcmp(buffer, "0") == 0){       // "0"表示对方拒收文件
            QMessageBox::information(this,tr("提示"),tr("对方拒绝接收文件"));
            closesocket(toUsr);
            ui->sendButton->setEnabled(true);
            return;
        }
        // 接收用户udp监听端口
        memcpy(&toAddr, buffer, sizeof(sockaddr_in));
        cout << inet_ntoa(toAddr.sin_addr) << endl;
        closesocket(toUsr);
    }
    else{
        cout << "error code:" << WSAGetLastError() << endl;
        QMessageBox::information(this,tr("提示"),tr("网络连接失败！"));
        closesocket(toUsr);
        ui->sendButton->setEnabled(true);
        return;
    }

    // 开始传输
    std::thread start_trans(&SendFile::send_file, this);
    start_trans.detach();
}

// 文件传输线程
void SendFile::send_file()
{
    // 绑定UDP端口
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    ::bind(udpSock, (LPSOCKADDR)&clientUdp, sizeof(sockaddr_in));

    base = nextseqnum = seqnum = 1;
    fp = fopen(filename.toStdString().c_str(), "rb");
    cout << "start to trans." << endl;

    // 将文件分段发送
    make_packet();

    // 用于接收确认报文
    std::thread ack(&SendFile::recv_ack, this);
    ack.detach();
}

// 接收确认ACK
void SendFile::recv_ack()
{
    connect(this, SIGNAL(set_bar_value()), this, SLOT(update_processbar()),Qt::QueuedConnection);

    long tmp;
    recvAck = 0;
    connect(this, SIGNAL(pop_qmessage()), this, SLOT(messageBox()));
    while(1){
        tmp = recvAck;
        recvfrom(udpSock, (char *)&recvAck, sizeof(long), 0, (LPSOCKADDR)&toAddr, &nAddr);
        cout << "recev ack " << recvAck << endl;
        emit set_bar_value();
        if(recvAck == maxseq){
            emit pop_qmessage();
            return;
        }
        base = recvAck + 1;
        // 根据回应ack决定执行哪种操作
        if(tmp == recvAck)
            resend();
        else{
            if(base == nextseqnum){
                if(timer->isActive())
                    emit stop_signal();
            }
            else emit start_signal();
            make_packet();
        }
    }
}

void SendFile::messageBox()
{
    QMessageBox::information(this,tr("提示"),tr("文件发送成功！"));
}

// 发送
void SendFile::make_packet()
{
    DataGram sendGram;

    while(nextseqnum < (base + NWINDOW) && seqnum <= maxseq){
        // 读取文件
        memset(&sendGram, 0, sizeof(DataGram));
        fread(sendGram.data, MAXLEN, 1, fp);
        sendGram.seqnum = seqnum++;
        dataqueue.push_back(sendGram);
        // 传数据
        cout << "seqnum " << sendGram.seqnum << endl;
        sendto(udpSock, (char *)&sendGram, sizeof(DataGram), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
        // 开启一个定时器
         if(base == nextseqnum)
             emit start_signal();
        ++nextseqnum;
    }
    if(seqnum == maxseq+1) fclose(fp);
    while(dataqueue[0].seqnum < base){
        dataqueue.pop_front();
        if(dataqueue.empty()) break;
    }
}

// 超时重传
void SendFile::resend()
{
    emit start_signal();
    // 重传
    for(auto i: dataqueue)
        sendto(udpSock, (char *)&i, sizeof(DataGram), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
}

// 更新进度条
void SendFile::update_processbar()
{
    ui->progressBar->setValue(recvAck*1.0/maxseq*100);
}

void SendFile::timer_start()
{
    timer->start(TIMEOUT);
}

void SendFile::timer_stop()
{
    timer->stop();
}

