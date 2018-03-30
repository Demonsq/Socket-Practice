#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(SOCKET &sock, sockaddr_in &addr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    toSrv(sock),
    srvAddr(addr)
{
    ui->setupUi(this);
    setWindowTitle("登录");
    ui->usrLineEdit->setFocus();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

// 进入注册界面
void LoginDialog::on_signupBtn_clicked()
{
    QDialog *signup_dlg = new SignupDialog(srvAddr, this);
    signup_dlg->exec();
}

// 进入找回密码界面
void LoginDialog::on_getpwdBtn_clicked()
{
    QDialog *getpwd_dlg = new GetpwdDialog(srvAddr, this);
    getpwd_dlg->exec();
}

// 处理用户登录
void LoginDialog::on_loginBtn_clicked()
{
    toSrv = socket(AF_INET, SOCK_STREAM, 0);        // 创建socket

    string user = ui->usrLineEdit->text().toStdString();
    string passward = ui->pwdLineEdit->text().toStdString();

    // 网络连接错误
    if(::connect(toSrv, (LPSOCKADDR)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR ||
            send(toSrv, "1", BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, user.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            send(toSrv, passward.c_str(), BUFLEN, 0) == SOCKET_ERROR ||
            recv(toSrv,recvBuf,BUFLEN,0) == SOCKET_ERROR) {
        QMessageBox::information(this,tr("提示"),tr("网络连接失败！"));
        closesocket(toSrv);
        return;
    }

    // 登录成功
    if(strcmp(recvBuf, "1") == 0) accept();
    // 登录失败
    else{
        closesocket(toSrv);
        QMessageBox::information(this,tr("提示"),tr(recvBuf));
        ui->pwdLineEdit->clear();
        ui->usrLineEdit->setFocus();
    }
}
