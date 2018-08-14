#include "editmatdb.h"
#include "ui_editmatdb.h"

EditMatDB::EditMatDB(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditMatDB)
{
    ui->setupUi(this);
}

EditMatDB::~EditMatDB()
{
    delete ui;
}
