#include "matdb.h"

MatDB::MatDB(QObject *parent)
    :QAbstractTableModel(parent)
{
    mat_list = new QList<mat>();
}

int MatDB::rowCount(const QModelIndex & /*parent*/) const
{
   return 2;
}

int MatDB::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant MatDB::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
       return QString("Row%1, Column%2")
                   .arg(index.row() + 1)
                   .arg(index.column() +1);
    }
    return QVariant();
}

QVariant MatDB::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Material Alt");
            case 1:
                return QString("Material Neu");
            case 2:
                return QString("Oberfläche");
            }
        }
    }
    return QVariant();
}
