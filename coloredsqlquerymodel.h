#ifndef COLOREDSQLQUERYMODEL_H
#define COLOREDSQLQUERYMODEL_H

#include <QObject>
#include <QSqlQueryModel>

class ColoredSqlQueryModel : public QSqlQueryModel
{
public:
    explicit ColoredSqlQueryModel(QObject *parent=nullptr);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // COLOREDSQLQUERYMODEL_H
