#ifndef EDITMATDB_H
#define EDITMATDB_H

#include <QDialog>

namespace Ui {
class EditMatDB;
}

class EditMatDB : public QDialog
{
    Q_OBJECT

public:
    explicit EditMatDB(QWidget *parent = 0);
    ~EditMatDB();

private:
    Ui::EditMatDB *ui;
};

#endif // EDITMATDB_H
