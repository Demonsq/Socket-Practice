#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "socket.h"
#include "sendfile.h"
#include "recvfile.h"
#include "chatwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SOCKET &sock, QWidget *parent = 0);

    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:

    void on_chatButton_clicked();

    void on_fileButton_clicked();

    void create_new_window(SOCKET &sock, const char *name0,
                           const char *name1, const char *buffer);

    void create_getfile_window(SOCKET &tcp);

signals:

    void request_new_window(SOCKET &sock, const char *name0,
                            const char *name1, const char *buffer);

    void request_getfile_window(SOCKET &tcp);

private:
    Ui::MainWindow *ui;

    int nRC;
    string name;
    SOCKET asSrv;
    SOCKET &toSrv;
    SOCKET udpSock;
    QString fileName;
    char sendBuf[BUFLEN];
    char recvBuf[BUFLEN];
    sockaddr_in clientAddr;
    ChatWindow *newWindow;
    vector<ChatWindow *> chat;

    void usrAccept();
};

#endif // MAINWINDOW_H
