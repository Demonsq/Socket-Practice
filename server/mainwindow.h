#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "server.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void confirm_thread(SrvInfo &srv, vector<UserInfo> &usr);
    void online_user(int id, SOCKET connSock, SrvInfo &srv, vector<UserInfo> &usr);
    ~MainWindow();

private slots:
    void update_log(const char *log);

    void update_usrList(int &id, int &flag);

    void addTolist(const char *user_name);

signals:
    void log_signal(const char *log);

    void usrList_signal(int &id, int &flag);

    void addUsrToList(const char *user_name);

private:
    Ui::MainWindow *ui;
    string log;
    SrvInfo srv;
    vector<UserInfo> usr;
};

#endif // MAINWINDOW_H
