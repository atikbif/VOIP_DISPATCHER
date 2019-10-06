#ifndef GROUP_H
#define GROUP_H

#include <QString>
#include <QTreeWidgetItem>
#include <optional>
#include <any>
#include "point.h"

class Group
{
    QString name;
    int real_point_cnt;
    Input di1,di2,di3;
    bool do1,do2;
    bool not_actual=false;
    std::vector<Point> points;
    QTreeWidgetItem *item=nullptr;
    QTreeWidgetItem *di1Item=nullptr;
    QTreeWidgetItem *di2Item=nullptr;
    QTreeWidgetItem *di3Item=nullptr;
    QTreeWidgetItem *do1Item=nullptr;
    QTreeWidgetItem *do2Item=nullptr;
    QTreeWidgetItem *cntItem=nullptr;
public:
    explicit Group(QTreeWidgetItem *item,const QString &name);
    void addNewPoint(const QString &name);
    void setPointValue(int pointNum, const QString &param, std::any value);
    std::optional<std::any> getPointValue(int pointNum, const QString &param);
    void setGroupValue(const QString &param, std::any value);
    std::optional<std::any> getGroupValue(const QString &param);
    int count() {return static_cast<int>(points.size());}
    void update(){}
};

#endif // GROUP_H
