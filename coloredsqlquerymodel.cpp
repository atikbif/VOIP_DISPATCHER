#include "coloredsqlquerymodel.h"
#include <QColor>
#include <QBrush>

ColoredSqlQueryModel::ColoredSqlQueryModel(QObject *parent):QSqlQueryModel (parent)
{

}

QVariant ColoredSqlQueryModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
       QString txt = data(index, Qt::DisplayRole).toString().toLower();
       if(txt.contains("авария")) return QBrush(QColor(255,0,0,150));
       else if(txt.contains("предупреждение")) return QBrush(QColor(255,255,0,150));
       else if(txt.contains("сообщение")) return QBrush(QColor(0,255,0,150));
    }
    return QSqlQueryModel::data(index, role);
}
