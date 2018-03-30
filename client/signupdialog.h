#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include "socket.h"

namespace Ui {
class SignupDialog;
}

class SignupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignupDialog(sockaddr_in &addr, QWidget *parent = 0);
    ~SignupDialog();

private slots:
    void on_signupBtn_clicked();

private:
    Ui::SignupDialog *ui;
    sockaddr_in srvAddr;
    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];
};

#endif // SIGNUPDIALOG_H
