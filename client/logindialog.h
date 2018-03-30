#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "socket.h"
#include "signupdialog.h"
#include "getpwddialog.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(SOCKET &sock, sockaddr_in &addr, QWidget *parent = 0);
    ~LoginDialog();

private slots:
    void on_signupBtn_clicked();

    void on_getpwdBtn_clicked();

    void on_loginBtn_clicked();

private:
    Ui::LoginDialog *ui;
    SOCKET &toSrv;
    sockaddr_in srvAddr;
    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];
};

#endif // LOGINDIALOG_H
