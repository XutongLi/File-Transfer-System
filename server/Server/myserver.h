#ifndef MYSERVER_H
#define MYSERVER_H
#include <QtNetwork>

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    MyServer();

signals:
    void showMessServerToWidget(QString);   //状态信息传递的信号

private slots:
    void getMessThreadToServer(QString);    //状态信息穿的槽

protected:
    virtual void incomingConnection(qintptr socketDescriptor);
};

#endif // MYSERVER_H
