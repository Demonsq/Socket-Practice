#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QDateTime>
#include "socket.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog
{
    Q_OBJECT

public:
    ChatWindow(SOCKET &sock, const char *name0, const char *name1, QWidget *parent);
    void displayMessage(const char *recvBuf);
    ~ChatWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_sendButton_clicked();

private:
    Ui::ChatWindow *ui;
    SOCKET toUsr;
    SOCKET &toSrv;
    QString time;
    string myname;
    sockaddr_in usrAddr;
    QDateTime currentTime;
    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];

public:
    string user_name;
};

#endif // CHATWINDOW_H
