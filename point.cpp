#include "point.h"
#include <QDebug>

Point::Point(QTreeWidgetItem *item, const QString &name):name(name),item(item)
{
    di1 = Input::OFF;
    di2 = Input::OFF;
    power = 0;
    battery = 0;
    do1 = false;
    do2 = false;
    if(item) {
        item->setText(0,name);
        speakerIitem = new QTreeWidgetItem(item,QStringList()<< "Динамики"<<"не известно");
        di1Item = new QTreeWidgetItem(item,QStringList()<< "Вход 1 (КТВ)"<<"не известно");
        di2Item = new QTreeWidgetItem(item,QStringList()<< "Вход 2"<<"не известно");
        do1Item = new QTreeWidgetItem(item,QStringList()<< "Выход 1 (разреш)"<<"не известно");
        do2Item = new QTreeWidgetItem(item,QStringList()<< "Выход 2 (предстарт)"<<"не известно");
        powerItem = new QTreeWidgetItem(item,QStringList()<< "Питание, В"<<"не известно");
        batteryItem = new QTreeWidgetItem(item,QStringList()<< "Аккумулятор, В"<<"не известно");
        versionItem = new QTreeWidgetItem(item,QStringList()<< "Версия"<<"не известно");
        volumeItem = new QTreeWidgetItem(item,QStringList()<< "Громкость"<<"не известно");
        limitSwitchItem = new QTreeWidgetItem(item,QStringList()<<"Концевик"<<"не известно");
        //connect(volumeItem,&QTreeWidgetItem::)
    }
}

std::optional<std::any> Point::getPointValue(const QString &param)
{
    if(param=="name") return std::make_optional<std::any>(name);
    else if(param=="speaker") return std::make_optional<std::any>(speaker);
    else if(param=="power") return std::make_optional<std::any>(power);
    else if(param=="battery") return std::make_optional<std::any>(battery);
    else if(param=="di1") return std::make_optional<std::any>(di1);
    else if(param=="di2") return std::make_optional<std::any>(di2);
    else if(param=="do1") return std::make_optional<std::any>(do1);
    else if(param=="do2") return std::make_optional<std::any>(do2);
    else if(param=="version") return std::make_optional<std::any>(version);
    else if(param=="volume") return std::make_optional<std::any>(volume);
    else if(param=="limit_switch") return std::make_optional<std::any>(limit_switch);
    return std::nullopt;
}

void Point::setPointValue(const QString &param, std::any value)
{
    if(value.has_value()) {
        if(param=="speaker") {
            speaker=std::any_cast<Speaker>(value);
            switch(speaker) {
                case Speaker::CORRECT:
                    speakerIitem->setTextColor(1, Qt::black);
                    speakerIitem->setText(1,"Исправны");
                    break;
                case Speaker::PROBLEM:
                    speakerIitem->setTextColor(1, Qt::red);
                    speakerIitem->setText(1,"Авария");
                    break;
                case Speaker::NOT_CHECKED:
                    speakerIitem->setTextColor(1, Qt::red);
                    speakerIitem->setText(1,"Не проверялись");
                    break;
            }

        }else if(param=="power") {
            power=std::any_cast<double>(value);
            powerItem->setText(1,QString::number(power));
        } else if(param=="battery") {
            battery=std::any_cast<double>(value);
            batteryItem->setText(1,QString::number(battery));
        }
        else if(param=="di1") {
            di1=std::any_cast<Input>(value);
            if(di1Item) {
                switch (di1) {
                    case Input::ON:
                        di1Item->setText(1,"ВКЛ");
                        di1Item->setTextColor(1, Qt::black);
                        break;
                    case Input::OFF:
                        di1Item->setText(1,"ВЫКЛ");
                        di1Item->setTextColor(1, Qt::red);
                        break;
                    case Input::BREAK:
                        di1Item->setText(1,"ОБРЫВ!");
                        di1Item->setTextColor(1, Qt::red);
                        break;
                    case Input::SHORT:
                        di1Item->setText(1,"КЗ");
                        di1Item->setTextColor(1, Qt::red);
                        break;
                }
            }
        }else if(param=="di2") {
            di2=std::any_cast<Input>(value);
            if(di2Item) {
                switch (di2) {
                    case Input::ON:di2Item->setText(1,"ВКЛ");break;
                    case Input::OFF:di2Item->setText(1,"ВЫКЛ");break;
                    case Input::BREAK:di2Item->setText(1,"ОБРЫВ!");break;
                    case Input::SHORT:di2Item->setText(1,"КЗ");break;
                }
            }
        }
        else if(param=="do1") {
            do1=std::any_cast<bool>(value);
            if(do1) do1Item->setText(1,"Вкл");
            else do1Item->setText(1,"Выкл");
        }
        else if(param=="do2") {
            do2=std::any_cast<bool>(value);
            if(do2) do2Item->setText(1,"Вкл");
            else do2Item->setText(1,"Выкл");
        }else if(param=="version") {
            version=std::any_cast<QString>(value);
            versionItem->setText(1,version);
        }else if(param=="volume") {
            volume=std::any_cast<QString>(value);
            volumeItem->setText(1,volume);
        }else if(param=="limit_switch") {
            limit_switch=std::any_cast<bool>(value);
            if(limit_switch) limitSwitchItem->setText(1,"Вкл");
            else limitSwitchItem->setText(1,"Выкл");
        }
    }
}

void Point::setPointToDefault()
{
    if(speakerIitem) speakerIitem->setText(1,"не известно");
    if(di1Item) di1Item->setText(1,"не известно");
    if(di2Item) di2Item->setText(1,"не известно");
    if(do1Item) do1Item->setText(1,"не известно");
    if(do2Item) do2Item->setText(1,"не известно");
    if(powerItem) powerItem->setText(1,"не известно");
    if(batteryItem) batteryItem->setText(1,"не известно");
    if(versionItem) versionItem->setText(1,"не известно");
    if(volumeItem) volumeItem->setText(1,"не известно");
    if(limitSwitchItem) limitSwitchItem->setText(1,"не известно");
}

void Point::update()
{

}
