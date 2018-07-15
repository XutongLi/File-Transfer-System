#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QMainWindow>

class QSqlTableModel;
namespace Ui {
class DataModel;
}

class DataModel : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataModel(QWidget *parent = 0);
    ~DataModel();

private slots:
    void on_submitButton_clicked();

    void on_rollbackButton_clicked();

    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::DataModel *ui;
    QSqlTableModel *model;      //数据库
};

#endif // DATAMODEL_H
