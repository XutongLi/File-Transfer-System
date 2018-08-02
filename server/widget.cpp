#include "widget.h"
#include "ui_widget.h"
#include "datamodel.h"
#include <QGridLayout>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //添加样式
    qssFile = new QFile(":/qss/myStyleSheetB.qss",this);
    qssFile->open(QFile::ReadOnly);
    QString styleSheet = tr(qssFile->readAll());
    qApp->setStyleSheet(styleSheet);
    qssFile->close();

    //栅格化布局
    setMyLayout();

    //建立服务器
    server = new MyServer();
    server->listen(QHostAddress::LocalHost,6666);   //开始监听

     //动态传递的信号与槽连接
    connect(server, SIGNAL(showMessServerToWidget(QString)), this, SLOT(showMessage(QString)));
}


Widget::~Widget()
{
    delete server;
    delete layout;
    delete sqlModel;
    delete ui;
}

/*栅格化布局*/
void Widget::setMyLayout()
{
    layout = new QGridLayout;
    layout->addWidget(ui->textBrowser,1,0,1,4);
    layout->addWidget(ui->pushButton,2,1,1,2);
    layout->setHorizontalSpacing(20);   //控件间横向距离
    layout->setVerticalSpacing(30);     //控件间纵向距离
    layout->setMargin(30);              //设置内边距
    setLayout(layout);
    this->resize(500,380);
}

/*打开用户信息*/
void Widget::on_pushButton_clicked()
{
    sqlModel = new DataModel();
    sqlModel->setWindowTitle("用户信息");
    sqlModel->resize(330,500);
    sqlModel->show();
}

/*服务器输出信息*/
void Widget::showMessage(QString message)
{
    ui->textBrowser->append(message);
}
