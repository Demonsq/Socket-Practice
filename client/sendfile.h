#ifndef SENDDATA_H
#define SENDDATA_H

#include "socket.h"
#include <deque>
#include <QTimer>
#include <QFileDialog>

namespace Ui {
class SendFile;
}

class SendFile : public QDialog
{
    Q_OBJECT

public:
    SendFile(sockaddr_in &addr, string &myname, string &anoUsr, QWidget *parent = 0);
    void send_file();       // 文件传输
    void recv_ack();        // 接收确认报文
    void make_packet();
    ~SendFile();

private slots:
    void resend();          // 超时重传

    void on_chooseButton_clicked();

    void on_sendButton_clicked();

    void update_processbar();

    void timer_start();

    void timer_stop();

    void messageBox();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void set_bar_value();

    void start_signal();

    void stop_signal();

    void pop_qmessage();

private:
    Ui::SendFile *ui;
    int nAddr;
    long recvAck;
    long filelen;
    long seqnum;
    long maxseq;
    long base;
    long nextseqnum;
    FILE *fp;
    QTimer *timer;
    string &name;
    SOCKET udpSock;
    QString filename;
    sockaddr_in toAddr;
    sockaddr_in clientUdp;
    sockaddr_in clientAddr;
    deque<DataGram> dataqueue;
};

#endif // SENDDATA_H
