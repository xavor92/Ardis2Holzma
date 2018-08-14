#include <QDebug>

#include "editmatdb.h"
#include "ui_editmatdb.h"

EditMatDB::EditMatDB(QWidget *parent, MatDB *mat_db) :
    QDialog(parent),
    ui(new Ui::EditMatDB)
{
    ui->setupUi(this);

    ui->tableView->setModel(mat_db);
    ui->tableView->show();

    QList<mat>::iterator iterate_obj = mat_db->mat_list->begin();
    for(iterate_obj = mat_db->mat_list->begin(); iterate_obj != mat_db->mat_list->end(); iterate_obj++)
    {
        qDebug() << "MAT:" << iterate_obj->matold << endl;
    }
}

EditMatDB::~EditMatDB()
{
    delete ui;
}
