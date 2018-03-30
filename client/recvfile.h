#ifndef RECVFILE_H
#define RECVFILE_H

#include "socket.h"

namespace Ui {
class RecvFile;
}

class RecvFile : public QDialog
{
    Q_OBJECT

public:
    explicit RecvFile(SOCKET &tcp, QWidget *parent = 0);
    void recv_file();
    ~RecvFile();

private slots:
    void on_cancelBtn_clicked();

    void on_getFileBtn_clicked();

    void update_processbar();

    void messageBox();

signals:
    void set_bar_value();

    void pop_qmessage();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::RecvFile *ui;
    int nAddr;
    long ackNum;
    long maxseq;
    long filesize;
    char filename[BUFLEN];
    string myname;
    SOCKET udpSock;
    SOCKET connSock;
    sockaddr_in toAddr;
    sockaddr_in clientAddr;

};

#endif // RECVFILE_H
