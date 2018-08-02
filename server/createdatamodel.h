#ifndef CREATEDATAMODEL_H
#define CREATEDATAMODEL_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>


/*建立数据库*/
static bool createDataModel()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user.db");
    if(!db.open()){
        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection.", QMessageBox::Cancel);
        return false;
    }
    QSqlQuery query;
    //数据库预设信息
    query.exec("create table user (id varchar primary key, pass varchar)");
    query.exec("insert into user values('李旭同', '2015211494')");
    query.exec("insert into user values('马云', '66666')");
    query.exec("insert into user values('马化腾', '23333')");
    return true;
}



#endif // CREATEDATAMODEL_H
