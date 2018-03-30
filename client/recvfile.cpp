#include "recvfile.h"
#include "ui_recvfile.h"

RecvFile::RecvFile(SOCKET &tcp, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecvFile),
    connSock(tcp)
{
    ui->setupUi(this);
    nAddr = sizeof(sockaddr_in);

    // 设置客户机信息（用于其他用户直接和本机连接）
    clientAddr.sin_family = AF_INET;		// 地址簇
    clientAddr.sin_port = htons(0);			// 系统自动分配端口号（1024-5000）
    clientAddr.sin_addr.S_un.S_addr = INADDR_ANY;       // 本机ip

    char name[BUFLEN];
    recv(connSock, name, BUFLEN, 0);
    recv(connSock, filename, BUFLEN, 0);
    recv(connSock, (char *)&filesize, sizeof(long), 0);     // 文件大小
    maxseq = filesize/MAXLEN + 1;                           // 序号大小
    myname = name;
    setWindowTitle(("From: " + myname).c_str());
    ui->fileName->setText(filename);
    QString size;
    size.setNum(filesize/1024);
    ui->fileSize->setText(size+"KB");
}

RecvFile::~RecvFile()
{
    delete ui;
}

void RecvFile::closeEvent(QCloseEvent *event)
{
    event->accept();
}

// 拒收文件
void RecvFile::on_cancelBtn_clicked()
{
    send(connSock, "0", BUFLEN, 0);
    closesocket(connSock);
    close();
}

// 接收文件
void RecvFile::on_getFileBtn_clicked()
{
    ui->getFileBtn->setEnabled(false);
    ui->cancelBtn->setEnabled(false);

    char buffer[BUFLEN];

    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    ::bind(udpSock, (LPSOCKADDR)&clientAddr, sizeof(sockaddr_in));
    bind(udpSock, (LPSOCKADDR)&clientAddr, sizeof(sockaddr_in));
    sockaddr_in temp;
    getsockname(connSock, (LPSOCKADDR)&temp, &nAddr);
    getsockname(udpSock, (LPSOCKADDR)&clientAddr, &nAddr);
    clientAddr.sin_addr = temp.sin_addr;
    // 将UDP监听端口发给对方
    memcpy(buffer, &clientAddr, sizeof(sockaddr_in));
    send(connSock, buffer, BUFLEN, 0);
    closesocket(connSock);

    // 接收文件
    std::thread start_recv(&RecvFile::recv_file, this);
    start_recv.detach();
}

void RecvFile::recv_file()
{
    ackNum = 1;
    DataGram recvGram;

    connect(this, SIGNAL(set_bar_value()), this, SLOT(update_processbar()), Qt::QueuedConnection);

    FILE *fp = fopen(filename, "wb");

    cout << "start to recv." << endl;
    long tmp=0;
    while(1){
        memset(&recvGram, 0, sizeof(DataGram));
        recvfrom(udpSock, (char *)&recvGram, sizeof(DataGram), 0, (LPSOCKADDR)&toAddr, &nAddr);
        if(ackNum == recvGram.seqnum){
            emit set_bar_value();
            cout << "recv seqnum " << recvGram.seqnum << endl;
            fwrite(recvGram.data, MAXLEN, 1, fp);
            sendto(udpSock, (char *)&ackNum, sizeof(long), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
            if(++ackNum == maxseq+1){
                connect(this, SIGNAL(pop_qmessage()), this, SLOT(messageBox()));
                emit pop_qmessage();
                cout << "trans succeed." << endl;
                break;
            }
        }
        else{
            tmp = ackNum - 1;
            sendto(udpSock, (char *)&tmp, sizeof(long), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
        }
//        if(len > 0){
//            if(ackNum == recvGram.seqnum){
//                emit set_bar_value();
//                cout << "recv seqnum " << recvGram.seqnum << endl;
//                fwrite(recvGram.data, MAXLEN, 1, fp);
//                sendto(udpSock, (char *)&ackNum, sizeof(long), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
//                ++ackNum;
//            }
//            else{
//                tmp = ackNum - 1;
//                sendto(udpSock, (char *)&tmp, sizeof(long), 0, (LPSOCKADDR)&toAddr, sizeof(sockaddr_in));
//            }
//        }
//        else if(len == 0){
//            connect(this, SIGNAL(pop_qmessage()), this, SLOT(messageBox()));
//            emit pop_qmessage();
//            cout << "trans succeed." << endl;
//            break;
//        }
//        else{
//            cout << "trans failed." << endl;
//            break;
//        }
    }
    fclose(fp);
    closesocket(udpSock);
    return;
}

void RecvFile::messageBox()
{
    QMessageBox::information(this,tr("提示"),tr("接收文件成功！"));
}

void RecvFile::update_processbar()
{
    ui->progressBar->setValue((ackNum*1.0/maxseq)*100);
}
