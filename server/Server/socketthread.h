#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H
#include <QtNetwork>
#include <QFile>

class SocketThread : public QThread
{
    Q_OBJECT
public:
    SocketThread();
    qintptr ptr;
    QTcpSocket *socket;
    void write_ptr(qintptr p){
        ptr = p;
    }

protected:
    virtual void run();

signals:
    void showMessThreadToServer(QString);       //传递状态信息的信号
    void startDownloadSignal();
    void hasWritten(qint64);

private slots:
    void startDownload();
    void updateDownloadProgress(qint64 numBytes);

private:
    bool ThreadRunning;

    const QString LOGOUT = "0";     //登出标记
    const QString LOGINREQ = "1";   //登录请求
    const QString REGIREQ = "2";    //注册请求
    const QString LOGINSUCCESS = "01";  //登录成功
    const QString LOGINPASSERROR = "02";    //登录密码错误
    const QString LOGINIDNON = "03";    //登录用户名不存在
    const QString REGISUCCESS = "11";   //注册成功
    const QString REGIIDBLANKERROR = "12"; //注册时ID为空错误
    const QString REGIPASSBLANKERROR = "13";    //注册时密码为空错误
    const QString REGIIDEXIT = "14";    //注册用户名已存在
    const QString UPDATE = "20";        //上传文件请求
    const QString UPSTART = "21";       //开始上传消息
    const QString UPREJ = "22";         //文件已存在，拒绝上传
    const QString SEARCH = "30";        //查找文件
    const QString FILEEXIST = "31";     //文件存在
    const QString FILENONEXIST = "32";  //文件不存在
    const QString FILENAMENON = "33";   //查找文件名为空
    const QString DOWNSTART = "34";     //开始下载
    const QString EXITAPP = "40";       //退出应用程序

    qint64 totalBytes;      //接收总大小
    qint64 bytesReceived;   //已接收大小
    qint64 fileNameSize;    //接收文件大小
    qint64 requestSize;       //用来存放接收到的请求的大小信息
    QString fileName;       //接收文件
    QFile *localFile;
    QByteArray inBlock;     //接收缓冲区
    bool isGettingFile;     //接收文件标记

    qint64 upTotalBytes;    //上传总大小
    qint64 upBytesReceived; //已上传大小
    qint64 upFileNameSize;  //上传文件名大小
    QString upFileName;     //上传文件名
    QFile *upLocalFile;     //上传文件
    QByteArray upBlock;     //上传缓冲区


    QString downFileName;       //要上传文件名
    qint64 downPerSize;       //传送文件时每次传送的大小
    qint64 downTotalBytes;  //下载总大小
    qint64 downBytesWritten;    //已下载大小
    qint64 downBytesToWrite;    //还要下载大小
    bool isDownloadingFile; //下载标记
    QFile *fileToDownload;    //上传的文件
    QByteArray  downBlock;  //下载缓冲区

    void sendMessToClient(QString Mess);    //向客户端发送消息
    void logoutExecution(QString str1);   //登出处理
    void loginExecution(QString str1, QString str2);    //处理登录请求
    void registerExecution(QString str1, QString str2); //处理注册请求
    void updateExecution(QString str1, QString str2);         //上传文件处理
    void searchExecution(QString str1, QString str2);   //查找请求处理
    void downStartExecution(QString str1, QString str2);    //开始下载处理
    void updateReject();    //文件已存在，拒绝上传
};

#endif // SOCKETTHREAD_H
