#include "matdb.h"

MatDB::MatDB(QObject *parent)
    :QAbstractTableModel(parent)
{
    mat_list = new QList<mat_t>();
}

int MatDB::rowCount(const QModelIndex & /*parent*/) const
{
   return mat_list->length();
}

int MatDB::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant MatDB::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    mat_t *mat = &(*mat_list)[row];

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(col)
        {
        case 0:
            return mat->matold;
            break;
        case 1:
            return mat->matnew;
            break;
        case 2:
            return mat->surface;
            break;
        }

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

bool MatDB::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        int row = index.row();
        int col = index.column();

        mat_t *mat = &(*mat_list)[row];

        switch(col)
        {
        case 0:
            mat->matold = value.toString();
            break;
        case 1:
            mat->matnew = value.toString();;
            break;
        case 2:
            mat->surface = value.toString();;
            break;
        }
    }
    return true;
}

Qt::ItemFlags MatDB::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}
