#include "client.h"
#include "ui_client.h"
#include <QGridLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QtNetwork>
#include <QFileDialog>
#include <QFile>

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    //添加样式
    qssFile = new QFile(":/qss/myStyleSheetB.qss",this);
    qssFile->open(QFile::ReadOnly);
    QString styleSheet = tr(qssFile->readAll());
    qApp->setStyleSheet(styleSheet);
    qssFile->close();

     //栅格化初始布局
    setMyLayout();
     //进入登录注册界面
    loginView();

    tcpClient = new QTcpSocket(this);
    tcpClient->connectToHost(QHostAddress::LocalHost,6666);

    //先不进行文件传输
    isUploadingFile = false;

    //接收文件初始化
    isDownloadingFile = false;
    downTotalBytes = 0;
    downBytesReceived = 0;
    downFileNameSize = 0;

    //当有可读数据时，发射readyread信号
    connect(tcpClient, &QTcpSocket::readyRead, this, &Client::readMessage);
    //连接错误
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(disPlayError(QAbstractSocket::SocketError)));//发生错误时error信号
    //发射开始上传信息好后开始上传
    connect(this, SIGNAL(startUploadSignal()), this, SLOT(startUpload()));
    //上传过程中更新上传进度
    connect(tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateUploadProgress(qint64)));

    qDebug() <<"ok";
}

Client::~Client()
{
    delete tcpClient;
    delete ui;
}

/*-------------------------页面布局模块--------------------------*/

/*栅格化初始布局*/
void Client::setMyLayout()
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ui->idLabel,1,0,1,2);
    layout->addWidget(ui->idLineEdit,1,2,1,4);
    layout->addWidget(ui->passLabel,2,0,1,2);
    layout->addWidget(ui->passLineEdit,2,2,1,4);
    layout->addWidget(ui->regiButton,3,0,1,3);
    layout->addWidget(ui->loginButton,3,3,1,3);
    layout->addWidget(ui->stateLabel,4,0,1,2);
    layout->addWidget(ui->contentLabel,4,2,1,4);
    layout->addWidget(ui->loginStatusLabel,5,0,1,4);
    layout->addWidget(ui->exitButton,5,4,1,2);
    layout->addWidget(ui->actLine,6,0,1,6);
    layout->addWidget(ui->upWarnLabel,7,2,1,2);
    layout->addWidget(ui->openButton,8,0,1,3);
    layout->addWidget(ui->upButton,8,3,1,3);
    layout->addWidget(ui->filenameLabel,9,0,1,6);
    layout->addWidget(ui->upBar,10,0,1,6);
    layout->addWidget(ui->sepLine,11,0,1,6);
    layout->addWidget(ui->downWarnLabel,12,2,1,2);
    layout->addWidget(ui->searchEdit,13,0,1,4);
    layout->addWidget(ui->searchButton,13,4,1,2);
    layout->addWidget(ui->downLabel,14,0,1,6);
    layout->addWidget(ui->downButton,15,0,1,3);
    layout->addWidget(ui->unDownButton,15,3,1,3);
    layout->addWidget(ui->downBar,16,0,1,6);
    layout->setHorizontalSpacing(20);   //控件间横向距离
    layout->setVerticalSpacing(30);     //控件间纵向距离
    layout->setMargin(30);              //设置内边距
    setLayout(layout);
}

/*登录界面*/
void Client::loginView()
{
    //要隐藏的控件
    ui->loginStatusLabel->hide();
    ui->exitButton->hide();
    ui->actLine->hide();
    ui->openButton->hide();
    ui->upButton->hide();
    ui->filenameLabel->hide();
    ui->upBar->hide();
    ui->sepLine->hide();
    ui->searchEdit->hide();
    ui->searchButton->hide();
    ui->downLabel->hide();
    ui->downButton->hide();
    ui->downBar->hide();
    ui->upWarnLabel->hide();
    ui->downWarnLabel->hide();
    ui->unDownButton->hide();
    //要显示的控件
    ui->idLabel->show();
    ui->idLineEdit->show();
    ui->passLabel->show();
    ui->passLineEdit->show();
    ui->loginButton->show();
    ui->regiButton->show();
    ui->stateLabel->show();
    ui->contentLabel->show();
    //设置大小
    this->resize(350,230);

}

/*客户端操作界面*/
void Client::actView()
{
    //要隐藏的控件
    ui->idLabel->hide();
    ui->idLineEdit->hide();
    ui->passLabel->hide();
    ui->passLineEdit->hide();
    ui->loginButton->hide();
    ui->regiButton->hide();
    ui->stateLabel->hide();
    ui->contentLabel->hide();
    //要显示的控件
    ui->loginStatusLabel->show();
    ui->actLine->show();
    ui->openButton->show();
    ui->upButton->show();
    ui->filenameLabel->show();
    ui->upBar->show();
    ui->sepLine->show();
    ui->searchEdit->show();
    ui->searchButton->show();
    ui->downLabel->show();
    ui->downButton->show();
    ui->downBar->show();
    ui->exitButton->show();
    ui->upWarnLabel->show();
    ui->downWarnLabel->show();
    ui->unDownButton->show();
    //控件初始化
    ui->downBar->reset();
    ui->upBar->reset();
    ui->filenameLabel->setText(tr("当前未选择文件"));
    ui->downLabel->setText(tr("当前未搜索文件"));
    ui->searchEdit->setText(tr(""));
    //上传下载按钮设置无效
    ui->upButton->setEnabled(false);
    ui->downButton->setEnabled(false);
    ui->unDownButton->setEnabled(false);
    //设置大小
    this->resize(350,420);
}


/*-------------------------页面布局模块结束--------------------------*/

/*-------------------------操作模块--------------------------------*/

/*发送消息的函数
 * Mess -- 消息指令
 * str1 -- 第一内容字符串
 * str2 -- 第二内容字符串
 * */
void Client::sendMessToServer(QString Mess, QString str1, QString str2){
    QByteArray requestBlock;    //用来存放请求信息
    QDataStream out(&requestBlock, QIODevice::WriteOnly);
    //设置数据流的版本，客户端和服务器使用的版本要相同
    out.setVersion(QDataStream::Qt_5_6);//版本
    out << (quint64)0;//因为在写入数据以前可能不知道实际数据的大小，所以要先在数据块的最前面留四个字节位置，以便以后填写数据大小
    out << Mess << str1 << str2;        //输入实际数据
    out.device()->seek(0);//跳转到数据块头部
    out << (quint64)(requestBlock.size() - sizeof(quint64));//填写信息大小
    tcpClient->write(requestBlock); //发送
    qDebug() <<"send" << Mess << str1 << str2;
    requestBlock.resize(0);
}

/*点击x按钮*/
void Client::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    if(isUploadingFile){    //如果正在传输文件，提示有文件正在上传
        button = QMessageBox::warning(this, tr("警告"), QString(tr("有文件正在上传，无法退出程序")),
                                      QMessageBox::Cancel);
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else if(isDownloadingFile){ //如果正在下载文件，提示有文件正在下载
        button = QMessageBox::warning(this, tr("警告"), QString(tr("有文件正在下载，无法退出程序")),
                                      QMessageBox::Cancel);
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else{
        button = QMessageBox::question(this, tr("退出程序"), QString(tr("确认退出程序?")),
                                      QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::No) {
            event->ignore();  //忽略退出信号，程序继续运行
        }
        else if (button == QMessageBox::Yes) {
            sendMessToServer(EXITAPP, MESSNULL, MESSNULL);  //向服务器发送退出客户端信息
            tcpClient->close();
            event->accept();  //接受退出信号，程序退出
            qDebug() <<"exit";
        }
    }
}

/*点击登录按钮，向服务器发送请求*/
void Client::on_loginButton_clicked()
{
    QString id = ui->idLineEdit->text();        //获取账户
    QString pass = ui->passLineEdit->text();    //获取密码
    sendMessToServer(LOGINREQ, id, pass);       //向服务器发送登录请求
}

/*点击注册按钮*/
void Client::on_regiButton_clicked()
{
    QString id = ui->idLineEdit->text();        //获取账户
    QString pass = ui->passLineEdit->text();    //获取密码
    sendMessToServer(REGIREQ, id, pass);        //向服务器发送注册请求
}

/*点击退出登录按钮*/
void Client::on_exitButton_clicked()
{
    QString id = ui->idLineEdit->text();        //获取账户
    loginView();    //显示登录界面
    ui->idLineEdit->setText("");    //登录输入行清空
    ui->passLineEdit->setText("");
    ui->contentLabel->setText(tr("请注册或登录"));
    sendMessToServer(LOGOUT, id, MESSNULL);     //向客户端发送退出登录请求
}

/*-------------------------操作模块结束--------------------------------*/

/*-------------------------接受信息模块-------------------------------*/

/*接收信息*/
void Client::readMessage()
{
    if(!isDownloadingFile){     //不是接收文件状态
        QString getMess;       //接收到的消息
        messageSize = 0;    //初始化信息大小
        QDataStream in(tcpClient);
        in.setVersion(QDataStream::Qt_5_6);
        if(messageSize == 0){   //如果是刚开始接收数据
            //判断接收的数据是否大于四字节，也就是文件的大小信息所占的空间
            //如果是则保存到blockSize变量中，否则直接返回，继续接收数据
            if(tcpClient->bytesAvailable() < (int)sizeof(quint64)) return;
            in >> messageSize;
        }
        //如果没有得到全部的数据，则返回，继续接收数据
        if(tcpClient->bytesAvailable() < messageSize) return;
        //将接收到的数据存放在变量中
        in >> getMess;
        qDebug() << "receive" << getMess;

        //根据接收信息进行处理
        /*登录处理*/
        if(getMess == LOGINSUCCESS){    //登录成功
            actView();      //登陆成功后显示处理界面
            QString statusShow = "用户：" + ui->idLineEdit->text() + " 正在登录";
            ui->loginStatusLabel->setText(statusShow);  //显示用户登录信息
        }
        else if(getMess == LOGINPASSERROR){ //登录密码错误
            ui->passLineEdit->setText("");  //密码框重置
            ui->contentLabel->setText(tr("密码错误，请重新输入"));
        }
        else if(getMess == LOGINIDNON){   //登录用户不存在
            ui->idLineEdit->setText("");    //输入框清空
            ui->passLineEdit->setText("");
            ui->contentLabel->setText(tr("用户不存在，请重新输入"));
        }

        /*注册处理*/
        else if(getMess == REGISUCCESS){      //注册成功
            actView();      //登陆成功后显示处理界面
            QString statusShow = "用户：" + ui->idLineEdit->text() + " 正在登录";
            ui->loginStatusLabel->setText(statusShow);  //显示用户登录信息
        }
        else if(getMess == REGIIDBLANKERROR){   //ID为空错误
            ui->idLineEdit->setText("");    //输入框清空
            ui->passLineEdit->setText("");
            ui->contentLabel->setText(tr("用户名不能为空"));
        }
        else if(getMess == REGIPASSBLANKERROR){ //密码为空错误
            ui->passLineEdit->setText("");
            ui->contentLabel->setText(tr("密码不能为空"));
        }
        else if(getMess == REGIIDEXIT){     //ID已存在
            ui->idLineEdit->setText("");    //输入框清空
            ui->passLineEdit->setText("");
            ui->contentLabel->setText(tr("用户名已存在"));
        }
        else if(getMess == UPSTART){    //开始上传文件
            qDebug() << "start update" << upFileName;
            upPerSize = 64*1024;   //64kb
            upTotalBytes = 0;
            upBytesWritten = 0;
            upBytesToWrite = 0;
            emit startUploadSignal(); //发送上传文件信号
        }
        else if(getMess == UPREJ){      //文件已存在，重新上传
            ui->filenameLabel->setText(tr("此文件已存在，请重新上传"));
            ui->openButton->setEnabled(true);
            ui->searchButton->setEnabled(true);
            ui->exitButton->setEnabled(true);
        }
        else if(getMess == FILEEXIST){  //查询的文件存在
            QString searchFileName = ui->searchEdit->text();
            showDownFileName = searchFileName;  //要下载的文件名是查找到的文件名
            int realNameIndex = searchFileName.lastIndexOf("/");
            QString realName = searchFileName.right(searchFileName.length ()-realNameIndex-1);
            QString labelShow = realName + " 存在";
            ui->downLabel->setText(labelShow);      //显示文件存在，接下来选择是否下载
            ui->downButton->setEnabled(true);
            ui->unDownButton->setEnabled(true);     //选择下载或不进行下载
            ui->exitButton->setEnabled(false);
            ui->openButton->setEnabled(false);
            ui->searchButton->setEnabled(false);
        }
        else if(getMess == FILENONEXIST){   //查询的文件不存在
            QString searchFileName = ui->searchEdit->text();    //获取要查找的文件名
            int realNameIndex = searchFileName.lastIndexOf("/");
            QString realName = searchFileName.right(searchFileName.length ()-realNameIndex-1);  //取真正文件名
            QString labelShow = realName + " 不存在";
            ui->downLabel->setText(labelShow);
        }
        else if(getMess == FILENAMENON){    //查询的文件名为空
            ui->downLabel->setText(tr("查询的文件名不能为空"));
        }
    }

    //接收文件传输状态
    else{
        QDataStream in(tcpClient);
        in.setVersion(QDataStream::Qt_5_6);
        //如果接收到的数据小于16个字节，保存到来的文件头结构
        if(downBytesReceived <= sizeof(qint64) * 2){
            if((tcpClient->bytesAvailable() >= sizeof(qint64) * 2) && (downFileNameSize == 0)){
                //接收数据总大小信息和文件名大小信息
                in >> downTotalBytes >> downFileNameSize;
                qDebug() << "downTotalBytes" << downTotalBytes << "downFileNameSize" << downFileNameSize;
                downBytesReceived += sizeof(qint64) * 2;
            }
            if((tcpClient->bytesAvailable() >= downFileNameSize) && (downFileNameSize != 0)){
                //接收文件名，并建立文件
                in >> downFileName;
                ui->downLabel->setText(tr("下载文件 %1").arg(downFileName));
                downFileName = "./files/" + downFileName;   //将下载文件装入files文件夹中
                qDebug() << downFileName;
                downBytesReceived += downFileNameSize;
                downLocalFile = new QFile(downFileName);
                if(!downLocalFile->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                    qDebug() << "Client: open file error!";
                    return;
                }
            }
            else{
                return;
            }
        }
        //文件未接收完时继续接收
        if(downBytesReceived < downTotalBytes){
            downBytesReceived += tcpClient->bytesAvailable();
            downBlock = tcpClient->readAll();
            downLocalFile->write(downBlock);    //写入文件
            downBlock.resize(0);
        }

        //更新进度条
        ui->downBar->setMaximum(downTotalBytes);
        ui->downBar->setValue(downBytesReceived);

        //接收数据完成时
        if(downBytesReceived == downTotalBytes){
            downLocalFile->close();
            downTotalBytes = 0;
            downBytesReceived = 0;
            downFileNameSize = 0;
            QString labelShow = "已完成下载 " + showDownFileName;
            ui->downLabel->setText(labelShow);
            ui->exitButton->setEnabled(true);
            ui->openButton->setEnabled(true);
            ui->searchButton->setEnabled(true);
            ui->searchEdit->setText(tr(""));
            isDownloadingFile = false;  //状态设置为不接收文件
        }
    }


}

/*接收信息错误处理*/
void Client::disPlayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
}

/*-------------------------接受信息模块结束-------------------------------*/

/*------------------------------上传模块------------------------------------*/

/*点击打开文件按钮*/
void Client::on_openButton_clicked()
{
    ui->upBar->reset();
    ui->filenameLabel->setText(tr("状态：等待打开文件！"));
    openFile();     //打开文件
}

/*打开要上传的文件*/
void Client::openFile()
{
    upFileName = QFileDialog::getOpenFileName(this);
    if(!upFileName.isEmpty()){
        ui->upButton->setEnabled(true);     //上传按钮生效
        int realNameIndex = upFileName.lastIndexOf("/");
        QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取真正文件名
        ui->filenameLabel->setText(tr("文件名： %1").arg(realName));
        qDebug() << upFileName;
    }
}

/*开始上传按钮*/
void Client::on_upButton_clicked()
{
    //给服务器发送上传请求
    QString id = ui->idLineEdit->text();
    sendMessToServer(UPDATE, id, upFileName);  //向服务器发送开始上传请求

    ui->upButton->setEnabled(false);    //文件开始传输时上传按钮失效
    ui->openButton->setEnabled(false);  //打开文件按钮失效
    ui->exitButton->setEnabled(false);  //退出登录按钮失效
    ui->searchButton->setEnabled(false);    //查找按钮失效
    ui->downButton->setEnabled(false);  //下载按钮失效
}

/*开始上传文件*/
void Client::startUpload()
{
    fileToUpdate = new QFile(upFileName);
    if(!fileToUpdate->open(QFile::ReadOnly)){
        qDebug() <<"client: open file error!";
        return;
    }
    //获取文件大小
    upTotalBytes = fileToUpdate->size();
    QDataStream sendOut(&upBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    QString currentFileName = upFileName.right(upFileName.size() - upFileName.lastIndexOf('/') - 1);
    //保留总大小信息空间、文件名大小信息空间，然后输入文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    //这里的总大小是总大小信息、文件名大小信息、文件名和实际文件大小的总和
    upTotalBytes += upBlock.size();  //要发送的整个数据的大小（文件头结构+实际文件大小）  放在数据流最开始，占用第一个qint(64)的空间
    sendOut.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    sendOut << upTotalBytes <<qint64((upBlock.size() - sizeof(qint64) * 2));
    //发送完文件头结构后剩余的数据大小
    upBytesToWrite = upTotalBytes - tcpClient->write(upBlock);
    upBlock.resize(0);  //outBlock是暂存数据的，最后要将其清零
    qDebug() <<"transenddd";
    isUploadingFile = true;     //修改正在上传文件标记
    int realNameIndex = upFileName.lastIndexOf("/");
    QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取真正文件名
    ui->filenameLabel->setText(tr("正在上传： %1").arg(realName));
}

/*文件传输过程*/
void Client::updateUploadProgress(qint64 numBytes)
{
    if(isUploadingFile){
        //已经发送数据的大小
        upBytesWritten += (int)numBytes;
        //如果已经发送了数据
        if(upBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            upBlock = fileToUpdate->read(qMin(upBytesToWrite, upPerSize));
            //发送完一次数据后还剩余数据的大小
            upBytesToWrite -= (int)tcpClient->write(upBlock);
            //清空发送缓冲区
            upBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            fileToUpdate->close();
        }
        //更新进度条
        ui->upBar->setMaximum(upTotalBytes);
        ui->upBar->setValue(upBytesWritten);
        //如果发送完毕
        if(upBytesWritten == upTotalBytes){
            int realNameIndex = upFileName.lastIndexOf("/");
            QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取真正文件名
            ui->filenameLabel->setText(tr("传送文件 %1 成功").arg(realName));
            fileToUpdate->close();
            isUploadingFile = false;    //退出文件传输状态
            ui->openButton->setEnabled(true);   //文件传输完成后打开文件按钮有效
            ui->exitButton->setEnabled(true);   //退出登录按钮有效
            ui->searchButton->setEnabled(true); //查找按钮有效
        }
    }
}

/*------------------------------上传模块结束----------------------------------*/

/*------------------------------下载模块-------------------------------------*/

/*查找服务器文件*/
void Client::on_searchButton_clicked()
{
    QString id = ui->idLineEdit->text();
    QString searchFileName = ui->searchEdit->text();    //获取要查找的文件名
    ui->downBar->reset();

    sendMessToServer(SEARCH, id, searchFileName);       //给服务器发送查找请求
}

/*查找到后进行下载的按钮*/
void Client::on_downButton_clicked()
{
    QString id = ui->idLineEdit->text();
    sendMessToServer(DOWNSTART, id, showDownFileName);  //给服务器发送下载请求

    isDownloadingFile = true;       //客户端进入接收下载文件状态
    ui->unDownButton->setEnabled(false);
    ui->downButton->setEnabled(false);
}

/*查找到后不进行下载的按钮*/
void Client::on_unDownButton_clicked()
{
    QString labelShow = "已取消下载 " + showDownFileName;
    ui->downLabel->setText(labelShow);
    ui->downButton->setEnabled(false);
    ui->unDownButton->setEnabled(false);
    ui->exitButton->setEnabled(true);
    ui->openButton->setEnabled(true);
    ui->searchButton->setEnabled(true);
    ui->searchEdit->setText(tr(""));
}
