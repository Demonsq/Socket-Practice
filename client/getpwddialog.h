#ifndef GETPWDDIALOG_H
#define GETPWDDIALOG_H

#include "socket.h"

namespace Ui {
class GetpwdDialog;
}

class GetpwdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetpwdDialog(sockaddr_in &addr, QWidget *parent = 0);
    ~GetpwdDialog();

private slots:
    void on_getpwdBtn_clicked();

private:
    Ui::GetpwdDialog *ui;
    sockaddr_in srvAddr;
    char recvBuf[BUFLEN];
};

#endif // GETPWDDIALOG_H
