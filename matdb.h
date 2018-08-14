#ifndef MATDB_H
#define MATDB_H

#include <QAbstractTableModel>
#include <QString>

//Material
typedef struct {
    QString matold;
    QString matnew;
    QString surface;
} mat_t;


class MatDB : public QAbstractTableModel
{
    Q_OBJECT
public:
    MatDB(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
/*  Qt::ItemFlags flags(const QModelIndex & index) const ; */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

/*  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);  */
    QList<mat_t> *mat_list;
};

#endif // MATDB_H
