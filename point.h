#ifndef POINT_H
#define POINT_H

#include <QString>
#include <QTreeWidgetItem>
#include <optional>
#include <any>

enum class Speaker{NOT_CHECKED,CORRECT,PROBLEM};
enum class Input{ON,OFF,SHORT,BREAK};

class Point
{
    QString name;
    Speaker speaker;
    double power;
    double battery;
    Input di1,di2;
    bool do1,do2;
    QTreeWidgetItem *item=nullptr;
    QTreeWidgetItem *speakerIitem=nullptr;
    QTreeWidgetItem *di1Item=nullptr;
    QTreeWidgetItem *di2Item=nullptr;
    QTreeWidgetItem *do1Item=nullptr;
    QTreeWidgetItem *do2Item=nullptr;
    QTreeWidgetItem *powerItem=nullptr;
    QTreeWidgetItem *batteryItem=nullptr;
public:
    explicit Point(QTreeWidgetItem *item,const QString &name);
    std::optional<std::any> getPointValue(const QString &param);
    void setPointValue(const QString &param, std::any value);
    void update();
};

#endif // POINT_H
