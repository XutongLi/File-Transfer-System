#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QAbstractSocket>

class QTcpSocket;
class QFile;

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private slots:
    void on_loginButton_clicked();
    void on_exitButton_clicked();
    void readMessage();
    void disPlayError(QAbstractSocket::SocketError);


    void on_regiButton_clicked();

    void on_openButton_clicked();

    void on_upButton_clicked();
    void startUpload();
    void updateUploadProgress(qint64 numBytes);

    void on_searchButton_clicked();

    void on_downButton_clicked();

    void on_unDownButton_clicked();

private:
    Ui::Client *ui;
    QFile *qssFile;
    //常量
    const QString LOGOUT = "0";     //登出标记
    const QString LOGINREQ = "1";        //登录标记
    const QString REGIREQ = "2";         //注册标记
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
    const QString MESSNULL = "-1";      //空信息

    //布局变量
    void setMyLayout();             //栅格化初始布局
    void loginView();               //登录界面
    void actView();                 //操作界面
    void closeEvent(QCloseEvent *event);    //点击x按钮
    void openFile();                //打开要上传的文件
    //网络文件变量
    QTcpSocket *tcpClient;
    qint64 messageSize;     //接收到的信息大小
    QString upFileName;       //要上传文件名
    qint64 upPerSize;       //传送文件时每次传送的大小
    qint64 upTotalBytes;    //上传的文件总大小
    qint64 upBytesWritten;  //已上传大小
    qint64 upBytesToWrite;  //要上传的大小
    bool isUploadingFile;   //上传标记
    QFile *fileToUpdate;    //上传的文件
    QByteArray  upBlock;    //上传缓冲区

    QString downFileName;   //下载文件的文件名
    QString showDownFileName;   //下载文件的文件名
    bool isDownloadingFile; //下载状态
    qint64 downTotalBytes;  //总共下载大小
    qint64 downBytesReceived;   //已下载大小
    qint64 downFileNameSize;    //下载文件名大小
    QFile *downLocalFile;   //下载文件名
    QByteArray downBlock;   //下载缓冲区

    void sendMessToServer(QString Mess, QString str1, QString str2);    //发送请求到服务器

signals:
    void startUploadSignal();         //开始上传的信号

};

#endif // CLIENT_H
