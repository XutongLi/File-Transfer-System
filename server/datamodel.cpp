#include "datamodel.h"
#include "ui_datamodel.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

DataModel::DataModel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataModel)
{
    ui->setupUi(this);
    //建立数据库模型
    model = new QSqlTableModel(this);
    model->setTable("user");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->tableView->setModel(model);
}

DataModel::~DataModel()
{
    delete model;
    delete ui;
}

/*提交修改按钮*/
void DataModel::on_submitButton_clicked()
{
    model->database().transaction();
    if(model->submitAll()){
        if(model->database().commit()){
            QMessageBox::information(this, tr("tableModel"), tr("数据修改成功！"));
        }
    }
    else{
        model->database().rollback();
        QMessageBox::warning(this, tr("tableModel"), tr("数据库错误：%1").arg(model->lastError().text()), QMessageBox::Ok);
    }
}

/*撤销修改(已提交的不能修改)*/
void DataModel::on_rollbackButton_clicked()
{
    model->revertAll();
}

/*添加记录按钮*/
void DataModel::on_addButton_clicked()
{
    int rowNum = model->rowCount();
    QString id = "name";
    model->insertRow(rowNum);
    model->setData(model->index(rowNum,0), id);
}

/*删除选中行按钮*/
void DataModel::on_deleteButton_clicked()
{
    int curRow = ui->tableView->currentIndex().row();
    model->removeRow(curRow);
    int ok = QMessageBox::warning(this, tr("删除当前行！"), tr("你确定删除当前行吗？"), QMessageBox::Yes, QMessageBox::No);
    if(ok == QMessageBox::No){
        model->revert();
    }
    else{
        model->submit();
    }
}
