#include "client.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client w;
    w.setWindowTitle("文件传输系统");
    QDir dir;
    if(!dir.exists("files"))
    {
        dir.mkdir("files");
    }

    w.show();

    return a.exec();
}
