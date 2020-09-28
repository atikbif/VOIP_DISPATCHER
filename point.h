#ifndef POINT_H
#define POINT_H

#include <QString>
#include <QTreeWidgetItem>
#include <optional>
#include <any>
#include "pointdata.h"


class Point
{
    QString name;
    Speaker speaker = Speaker::NOT_CHECKED;
    double power;
    double battery;
    Input di1,di2;
    bool do1,do2;
    bool limit_switch;
    double di1Filter;
    double di2Filter;
    QString di2Type;
    QString version = "не известно";
    QString volume = "не известно";
    QTreeWidgetItem *item=nullptr;
    QTreeWidgetItem *speakerIitem=nullptr;
    QTreeWidgetItem *di1Item=nullptr;
    QTreeWidgetItem *di2Item=nullptr;
    QTreeWidgetItem *do1Item=nullptr;
    QTreeWidgetItem *do2Item=nullptr;
    QTreeWidgetItem *powerItem=nullptr;
    QTreeWidgetItem *batteryItem=nullptr;
    QTreeWidgetItem *versionItem=nullptr;
    QTreeWidgetItem *volumeItem=nullptr;
    QTreeWidgetItem *limitSwitchItem=nullptr;
    QTreeWidgetItem *di1FilterItem=nullptr;
    QTreeWidgetItem *di2FilterItem=nullptr;
    QTreeWidgetItem *di2TypeItem=nullptr;
public:
    explicit Point(QTreeWidgetItem *item,const QString &name);
    Point(const Point &obj) = default;
    Point& operator=(const Point &obj) = default;
    std::optional<std::any> getPointValue(const QString &param);
    void setPointValue(const QString &param, std::any value);
    void setPointToDefault();
    void update();
};

#endif // POINT_H
