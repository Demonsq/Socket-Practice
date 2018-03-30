#include "getpwddialog.h"
#include "ui_getpwddialog.h"

GetpwdDialog::GetpwdDialog(sockaddr_in &addr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetpwdDialog),
    srvAddr(addr)
{
    ui->setupUi(this);
    setWindowTitle("找回密码");
    ui->usrLineEdit->setFocus();
}

GetpwdDialog::~GetpwdDialog()
{
    delete ui;
}

void GetpwdDialog::on_getpwdBtn_clicked()
{
    SOCKET toSrv;
    toSrv = socket(AF_INET, SOCK_STREAM, 0);        // 创建socket

    string user = ui->usrLineEdit->text().toStdString();
    string tel = ui->telLineEdit->text().toStdString();

    if(::connect(toSrv, (LPSOCKADDR)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR ||
            send(toSrv, "3", BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, user.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, tel.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            recv(toSrv,recvBuf,BUFLEN,0) == SOCKET_ERROR) {
        QMessageBox::information(this,tr("提示"),tr("网络连接失败!"));
        closesocket(toSrv);
        return;
    }

    if(strcmp(recvBuf, "3") == 0){
        recv(toSrv,recvBuf,BUFLEN,0);
        closesocket(toSrv);
        QMessageBox::information(this,tr("提示"),tr(recvBuf));
        close();
    }
    else {
        closesocket(toSrv);
        QMessageBox::information(this,tr("提示"),tr(recvBuf));
    }
}
