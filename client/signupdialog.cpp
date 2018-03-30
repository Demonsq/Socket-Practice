#include "signupdialog.h"
#include "ui_signupdialog.h"

SignupDialog::SignupDialog(sockaddr_in &addr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignupDialog),
    srvAddr(addr)
{
    ui->setupUi(this);
    setWindowTitle("注册");
    ui->usrLineEdit->setFocus();
}

SignupDialog::~SignupDialog()
{
    delete ui;
}

void SignupDialog::on_signupBtn_clicked()
{   
    string user = ui->usrLineEdit->text().toStdString();
    string tel = ui->telLineEdit->text().toStdString();
    string passward = ui->pwdLineEdit->text().toStdString();
    string confirm = ui->confirmLineEdit->text().toStdString();

    if(user.empty() || tel.empty() || passward.empty()){
        QMessageBox::information(this,tr("提示"),tr("输入不能为空！"));
        return;
    }
    if(tel.length() != 11){
        QMessageBox::information(this,tr("提示"),tr("请输入正确的手机号！"));
        ui->telLineEdit->clear();
        ui->telLineEdit->setFocus();
        return;
    }
    if(passward.length() < 6){
        QMessageBox::information(this,tr("提示"),tr("密码长度不得少于6位"));
        ui->pwdLineEdit->clear();
        ui->pwdLineEdit->setFocus();
        return;
    }
    if(passward != confirm){
        QMessageBox::information(this,tr("提示"),tr("两次输入密码不一致"));
        ui->confirmLineEdit->clear();
        ui->confirmLineEdit->setFocus();
        return;
    }

    SOCKET toSrv;
    toSrv = socket(AF_INET, SOCK_STREAM, 0);        // 创建socket
    if(::connect(toSrv, (LPSOCKADDR)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR ||
            send(toSrv, "2", BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, user.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, passward.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, tel.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            recv(toSrv,recvBuf,BUFLEN,0) == SOCKET_ERROR) {
        QMessageBox::information(this,tr("提示"),tr("网络连接失败!"));
        closesocket(toSrv);
        return ;
    }

    if(strcmp(recvBuf, "2") == 0){
        closesocket(toSrv);
        QMessageBox::information(this,tr("提示"),tr("注册成功!"));
        close();
    }
    else{
        closesocket(toSrv);
        QMessageBox::information(this,tr("提示"),tr(recvBuf));
    }
}
