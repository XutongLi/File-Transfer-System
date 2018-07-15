#include "widget.h"
#include "datamodel.h"
#include "createdatamodel.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("服务器");

    QDir dir;
    if(!dir.exists("files"))
    {
        dir.mkdir("files");
    }


    //建立数据库
    if(!createDataModel()) return 1;

    w.show();
    return a.exec();
}
