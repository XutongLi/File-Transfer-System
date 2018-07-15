#ifndef DATAEXECUTION_H
#define DATAEXECUTION_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

/*向数据库中添加用户信息*/
void addDataModel(QString id, QString pass)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("insert into user (id, pass) values(?, ?)");
    query.addBindValue(id);
    query.addBindValue(pass);
    query.exec();
}

/*登录时查询数据库*/
/*返回值
 * 1 -- 登录成功
 * 2 -- 登录失败，密码错误
 * 3 -- 登录失败，无此用户*/
int loginSearchDataModel(QString id, QString pass)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.exec("select * from user");
    while(query.next()){
        if(query.value(0).toString() == id){
            if(query.value(1).toString() == pass){
                return 1;   //登录成功
            }
            else{
                return 2;   //密码错误
            }
        }
    }
    return 3;   //登录失败，无此用户
}

/*注册时查询数据库*/
/*返回值
 *true -- 用户名不存在，可以注册
 *false -- 用户名已存在
 * */
bool regiSearchDataModel(QString id)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.exec("select * from user");
    while(query.next()){
        if(query.value(0).toString() == id){
            return false;
        }
    }
    return true;
}

#endif // DATAEXECUTION_H
