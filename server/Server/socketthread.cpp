#include "socketthread.h"
#include "dataexecution.h"
#include "widget.h"


SocketThread::SocketThread()
{

}

/*线程开始运行*/
void SocketThread::run()
{
    qDebug() <<"start";
    isGettingFile = false;  //开始不接收文件
    isDownloadingFile = false;

    //建立socket
    socket = new QTcpSocket();
    socket->setSocketDescriptor(ptr);
    connect(this, SIGNAL(startDownloadSignal()), this, SLOT(startDownload()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateDownloadProgress(qint64)));

    //文件传输的初始化
    upTotalBytes = 0;
    upBytesReceived = 0;
    upFileNameSize = 0;


    //循环接收事件
    ThreadRunning = true;
    while(ThreadRunning){
        //等待接收事件
        if(!isGettingFile){   //接收请求
            requestSize = 0;
            socket->waitForReadyRead(-1);
            QString requeType;      //接收请求类型
            QString str1;           //接收第一个字符串
            QString str2;           //接收第二个字符串
            QDataStream in(socket);
            //设置数据流版本，这里要和服务器端相同
            in.setVersion(QDataStream::Qt_5_6);
            if(requestSize == 0){//如果是刚开始接收数据
                //判断接收的数据是否大于四字节，也就是文件的大小信息所占的空间
                //如果是则保存到requestSize变量中，否则直接返回，继续接收数据
                if(socket->bytesAvailable() < (int)sizeof(quint64)) return;
                in >> requestSize;
            }
            //如果没有得到全部的数据，则返回，继续接收数据
            if(socket->bytesAvailable() < requestSize) return;
            //将接收到的数据存放在变量中
            in >> requeType >> str1 >> str2;
            //显示接收到的数据
            qDebug() <<requeType << str1 << str2;

            //对收到的请求进行处理
            if(requeType == LOGOUT){       //登出
                logoutExecution(str1);
            }
            else if(requeType == LOGINREQ){  //登录请求
                loginExecution(str1, str2);     //处理登录请求
            }
            else if(requeType == REGIREQ){  //注册请求
                registerExecution(str1, str2);
            }
            else if(requeType == UPDATE){   //上传请求
                int realNameIndex = str2.lastIndexOf("/");
                QString realName = str2.right(str2.length ()-realNameIndex-1);  //取真正文件名
                QString searchFileName = "./files/" + realName;
                QFile file(searchFileName);
                if(file.exists()){      //文件存在，拒绝上传
                    qDebug() << searchFileName << tr("存在");
                    updateReject();
                    QString showMess = "[-拒绝上传-]服务器拒绝" + str1 + "上传" + realName;
                    emit showMessThreadToServer(showMess);
                }
                else{       //文件不存在
                    qDebug() << searchFileName << tr("不存在");
                    //允许客户端上传
                    updateExecution(str1, str2);
                }

            }
            else if(requeType == SEARCH){   //查找请求
                searchExecution(str1, str2);
            }
            else if(requeType == DOWNSTART){        //开始发送文件
                downStartExecution(str1, str2);
            }
            else if(requeType == EXITAPP){  //退出应用程序
                socket->close();    //关闭socket
                ThreadRunning = false;       //关闭此线程
            }
        }//if
        else{   //文件传输
            socket->waitForReadyRead(-1);
            QDataStream in(socket);
            in.setVersion(QDataStream::Qt_5_6);
            //如果接收到的数据小于16个字节，保存到来的文件头结构
            if(upBytesReceived <= sizeof(qint64) * 2){
                if((socket->bytesAvailable() >= sizeof(qint64) * 2) && (upFileNameSize == 0)){
                    //接收数据总大小信息和文件名大小信息
                    in >> upTotalBytes >> upFileNameSize;
                    upBytesReceived += sizeof(qint64) * 2;
                }
                if((socket->bytesAvailable() >= upFileNameSize) && (upFileNameSize != 0)){
                    //接收文件名，并建立文件
                    in >> upFileName;
                    upFileName = "./files/" + upFileName;   //将上传文件装入files文件夹中
                    qDebug() << upFileName;
                    upBytesReceived += upFileNameSize;
                    upLocalFile = new QFile(upFileName);
                    if(!upLocalFile->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                        qDebug() << "Server: open file error!";
                        return;
                    }
                }
                else{
                    return;
                }
            }
            //文件未接收完时继续接收
            if(upBytesReceived < upTotalBytes){
                upBytesReceived += socket->bytesAvailable();
                upBlock = socket->readAll();
                upLocalFile->write(upBlock);
                upBlock.resize(0);
            }

            //接收数据完成时
            if(upBytesReceived == upTotalBytes){
                upLocalFile->close();
                upTotalBytes = 0;
                upBytesReceived = 0;
                upFileNameSize = 0;
                isGettingFile = false;  //状态设置为不接收文件
            }
        }//if
    }//while
}

/*向客户端发送消息*/
void SocketThread::sendMessToClient(QString Mess)
{
    QByteArray sendBlock;    //用来存放请求信息
    sendBlock.resize(0);
    QDataStream out(&sendBlock, QIODevice::WriteOnly);
    //设置数据流的版本，客户端和服务器使用的版本要相同
    out.setVersion(QDataStream::Qt_5_6);//版本
    out << (quint64)0;//因为在写入数据以前可能不知道实际数据的大小，所以要先在数据块的最前面留八个字节的位置，以便以后填写数据大小
    out << Mess;      //输入实际数据
    out.device()->seek(0);//跳转到数据块头部
    out << (quint64)(sendBlock.size() - sizeof(quint64));//填写信息大小
    socket->write(sendBlock); //发送
    qDebug() << "send" << Mess;
}

/*登出处理*/
void SocketThread::logoutExecution(QString str1)
{
    QString id = str1;
    //抛出界面输出信息的信号
    QString showMess = "[-登出-]" + id + "登出成功";
    emit showMessThreadToServer(showMess);
}

/*处理登录请求*/
void SocketThread::loginExecution(QString str1, QString str2)
{
    QString id = str1;
    QString pass = str2;
    int result = loginSearchDataModel(id, pass);
    if(result == 1){        //登录成功
        qDebug() << id <<"登录成功";

        //抛出界面输出信息的信号
        QString showMess = "[-登录-]" + id + "登录成功";
        emit showMessThreadToServer(showMess);

        sendMessToClient(LOGINSUCCESS);     //向客户端发送登录成功信息
    }
    else if(result == 2){   //登录失败，密码错误
        qDebug() << id <<"密码错误";
        sendMessToClient(LOGINPASSERROR);   //向客户端发送登录密码错误的信息
    }
    else{                   //登录失败，查无此人
        qDebug() << id <<"查无此人";
        sendMessToClient(LOGINIDNON);       //向客户端发送查无此人的信息
    }
}

/*处理注册请求*/
void SocketThread::registerExecution(QString str1, QString str2)
{
    QString id = str1;
    QString pass = str2;
    bool exit = regiSearchDataModel(str1);    //查询是否重复，已重复返回false
    if(id == ""){   //ID为空
        sendMessToClient(REGIIDBLANKERROR);     //向客户端发送ID为空信息
    }
    else if(pass == ""){    //密码为空
        sendMessToClient(REGIPASSBLANKERROR);   //向客户端发送密码为空的信息
    }
    else if(exit == false){     //ID已存在
        sendMessToClient(REGIIDEXIT);           //向客户端发送ID已存在信息
    }
    else{       //注册成功
        sendMessToClient(REGISUCCESS);          //向客户端发送注册成功信息
        addDataModel(id, pass); //将注册信息添加到数据库

        QString showMess = "[-注册-]" + id + "注册并登录成功";   //抛出界面输出信息的信号
        emit showMessThreadToServer(showMess);
    }

}

/*处理上传请求*/
void SocketThread::updateExecution(QString str1, QString str2)
{
    QString id = str1;
//    QString upName = str2;
    int realNameIndex = str2.lastIndexOf("/");
    QString realName = str2.right(str2.length ()-realNameIndex-1);  //取真正文件名
    QString showMess = "[-上传文件-]" + id + "上传" + realName;
    emit showMessThreadToServer(showMess);

    isGettingFile = true;   //设置为接收文件标记
    sendMessToClient(UPSTART);  //向客户端发送开始上传信息
}

/*文件已存在，拒绝上传请求*/
void SocketThread::updateReject()
{
    sendMessToClient(UPREJ);    //向客户端发送拒绝上传信息
}

/*查找请求处理*/
void SocketThread::searchExecution(QString str1, QString str2)
{
    QString id = str1;
    if(str2 == ""){     //如果查找的文件名为空
        sendMessToClient(FILENAMENON);  //向客户端发送文件名为空信息
    }
    else{       //查找的文件名不为空
        QString searchFileName = "./files/" + str2;
        QFile file(searchFileName);
        if(file.exists()){      //文件存在
            qDebug() << searchFileName << tr("存在");
            sendMessToClient(FILEEXIST);    //向客户端发送文件训在信息
        }
        else{       //文件不存在
            qDebug() << searchFileName << tr("不存在");
            sendMessToClient(FILENONEXIST); //向客户端发送文件不存在信息
        }
    }
}

/*开始下载处理*/
void SocketThread::downStartExecution(QString str1, QString str2)
{
    QString id = str1;
    QString dFileName = str2;
    downFileName = "./files/" + dFileName;
    QString showMess = "[-下载-]" + id + "下载" + dFileName;
    emit showMessThreadToServer(showMess);

    qDebug() << "start download" << dFileName;
    downPerSize = 64*1024;   //64kb
    downTotalBytes = 0;
    downBytesWritten = 0;
    downBytesToWrite = 0;
    emit startDownloadSignal(); //发送上传文件信号
    qDebug() << "here";
}

/*开始下载文件时服务器开始传输*/
void SocketThread::startDownload()
{
    fileToDownload = new QFile(downFileName);
    if(!fileToDownload->open(QFile::ReadOnly)){
        qDebug() <<"server: open file error!";
        return;
    }
    //获取文件大小
    downTotalBytes = fileToDownload->size();
    QDataStream sendOut(&downBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    QString currentFileName = downFileName.right(downFileName.size() - downFileName.lastIndexOf('/') - 1);
    //保留总大小信息空间、文件名大小信息空间，然后输入文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    //这里的总大小是总大小信息、文件名大小信息、文件名和实际文件大小的总和
    downTotalBytes += downBlock.size();  //要发送的整个数据的大小（文件头结构+实际文件大小）  放在数据流最开始，占用第一个qint(64)的空间
    sendOut.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    sendOut << downTotalBytes <<qint64((downBlock.size() - sizeof(qint64) * 2));
    //发送完文件头结构后剩余的数据大小

    downBytesToWrite = downTotalBytes - socket->write(downBlock);
    downBlock.resize(0);//outBlock是暂存数据的，最后要将其清零
    qDebug() <<"trans";
    isDownloadingFile = true;
}

/*文件下载时服务器传输过程*/
void SocketThread::updateDownloadProgress(qint64 numBytes)
{
    if(isDownloadingFile){
        //已经发送数据的大小
        downBytesWritten += (int)numBytes;
        //如果已经发送了数据
        if(downBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            downBlock = fileToDownload->read(qMin(downBytesToWrite, downPerSize));
            //发送完一次数据后还剩余数据的大小

            downBytesToWrite -= (int)socket->write(downBlock);
            //emit hasWritten(haswrite);

            //socket->waitForBytesWritten();
            //清空发送缓冲区
            downBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            fileToDownload->close();
        }
        //如果发送完毕
        if(downBytesWritten == downTotalBytes){
            fileToDownload->close();
            isDownloadingFile = false;    //退出文件传输状态
        }
    }
}

