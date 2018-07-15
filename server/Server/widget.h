#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QGridLayout>
#include "myserver.h"
#include "datamodel.h"


class QFile;
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);

    ~Widget();

private slots:
    void on_pushButton_clicked();
    void showMessage(QString message);  //显示状态信息的槽


private:
    Ui::Widget *ui;
    QFile *qssFile;
    MyServer *server;
    QGridLayout *layout;
    DataModel *sqlModel;
    void setMyLayout();
};


#endif // WIDGET_H
