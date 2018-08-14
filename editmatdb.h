#ifndef EDITMATDB_H
#define EDITMATDB_H

#include <QDialog>
#include <QList>

#include "matdb.h"

namespace Ui {
class EditMatDB;
}

class EditMatDB : public QDialog
{
    Q_OBJECT

public:
    explicit EditMatDB(QWidget *parent = 0, MatDB *mat_db = 0);
    ~EditMatDB();

private:
    Ui::EditMatDB *ui;
    MatDB *mat_db_new;
};

#endif // EDITMATDB_H
