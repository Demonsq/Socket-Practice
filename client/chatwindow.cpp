#include "chatwindow.h"
#include "ui_chatwindow.h"

// 主动创建窗口
ChatWindow::ChatWindow(SOCKET &sock, const char *name0, const char *name1, QWidget *parent):
    QDialog(parent),
    ui(new Ui::ChatWindow),
    toSrv(sock),
    myname(name0),
    user_name(name1)
{
    ui->setupUi(this);
    setWindowTitle(("To: " + user_name).c_str());
    ui->inputText->setFocus();
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::closeEvent(QCloseEvent *event){
    event->ignore();
    this->hide();
}

void ChatWindow::on_sendButton_clicked()
{
    // 输入为空或输入过长
    if(ui->inputText->toPlainText().toStdString().empty()) return;
    if(ui->inputText->toPlainText().toStdString().size() > 200){
        QMessageBox::information(this,tr("提示"),tr("输入长度大于两百字节！"));
        return;
    }

    toUsr = socket(AF_INET, SOCK_STREAM, 0);
    send(toSrv, "2", BUFLEN, 0);
    send(toSrv, user_name.c_str(), BUFLEN, 0);
    // 接收用户TCP监听地址
    recv(toSrv, recvBuf, BUFLEN, 0);
    memcpy(&usrAddr, recvBuf, sizeof(sockaddr_in));

    // 添加发送时间及用户名
    if(::connect(toUsr, (LPSOCKADDR)&usrAddr, sizeof(sockaddr_in)) != SOCKET_ERROR){
        send(toUsr, "1", BUFLEN, 0);        // "1"表示发送给用户的是聊天消息
        currentTime = QDateTime::currentDateTime();
        time = currentTime.toString("yyyy-MM-dd hh:mm:ss");
        send(toUsr, (myname + " " + time.toStdString() + "\n" +
                     ui->inputText->toPlainText().toStdString()).c_str(), BUFLEN, 0);
        closesocket(toUsr);
    }
    else{
        // 发送离线消息
        send(toSrv, "3", BUFLEN, 0);
        send(toSrv, user_name.c_str(), BUFLEN, 0);
        displayMessage("对方处于离线，消息将保存到服务器");
        currentTime = QDateTime::currentDateTime();
        time = currentTime.toString("yyyy-MM-dd hh:mm:ss");
        send(toSrv, (myname + " " + time.toStdString() + "\n" +
                     ui->inputText->toPlainText().toStdString()).c_str(), BUFLEN, 0);
    }
    displayMessage((myname + " " + time.toStdString()
                    + "\n" + ui->inputText->toPlainText().toStdString()).c_str());
    displayMessage("\n");
    ui->inputText->clear();
    ui->inputText->setFocus();
}

void ChatWindow::displayMessage(const char *recvBuf)
{
    ui->MessageBrowser->append(recvBuf);
    ui->MessageBrowser->moveCursor(QTextCursor::End);
    ui->inputText->setFocus();
}
