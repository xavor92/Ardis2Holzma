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
    ui->tableView->resizeColumnsToContents();
}

EditMatDB::~EditMatDB()
{
    delete ui;
}
