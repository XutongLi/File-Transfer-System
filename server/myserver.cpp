#include "myserver.h"
#include "socketthread.h"

MyServer::MyServer()
{

}

/*新的连接到来时调用*/
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() <<"new connect" << socketDescriptor;
    SocketThread *thread = new SocketThread();  //每来一个连接建立一个线程
    thread->write_ptr(socketDescriptor);
    thread->moveToThread(thread);
    thread->start();
    //动态传递的信号与槽连接
    connect(thread, SIGNAL(showMessThreadToServer(QString)), this, SLOT(getMessThreadToServer(QString)));
}

/*槽：从线程接收信息到服务器*/
void MyServer::getMessThreadToServer(QString message)
{
    //抛出从服务器到界面传递的信号
    emit showMessServerToWidget(message);
}
