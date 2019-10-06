#include "group.h"

Group::Group(QTreeWidgetItem *item, const QString &name):name(name),item(item)
{
    points.reserve(100);
    if(item) {
        item->setText(0,name);
        cntItem = new QTreeWidgetItem(item,QStringList()<< "Число подкл. точек"<<"не известно");
        di1Item = new QTreeWidgetItem(item,QStringList()<< "Вход 1 (разреш. с предыд.)"<<"не известно");
        di2Item = new QTreeWidgetItem(item,QStringList()<< "Вход 2 (предпуск.)"<<"не известно");
        di3Item = new QTreeWidgetItem(item,QStringList()<< "Вход 3 (подтвержд. запуска)"<<"не известно");
        do1Item = new QTreeWidgetItem(item,QStringList()<< "Выход 1 (сигнал)"<<"не известно");
        do2Item = new QTreeWidgetItem(item,QStringList()<< "Выход 2 (готовность)"<<"не известно");
    }
}

void Group::addNewPoint(const QString &name)
{
    if(item) {
        QTreeWidgetItem *pointItem = new QTreeWidgetItem(item);
        points.push_back(Point(pointItem,name));
    }
}

void Group::setPointValue(int pointNum, const QString &param, std::any value)
{
    if(pointNum>=0 && pointNum<static_cast<int>(points.size())) {
        points[static_cast<std::vector<Point>::size_type>(pointNum)].setPointValue(param,value);
    }
}

std::optional<std::any> Group::getPointValue(int pointNum, const QString &param)
{
    if(pointNum>=0 && pointNum<static_cast<int>(points.size())) {
        return points[static_cast<std::vector<Point>::size_type>(pointNum)].getPointValue(param);
    }
    return std::nullopt;
}

void Group::setGroupValue(const QString &param, std::any value)
{
    if(value.has_value()) {
        if(param=="di1") {
            di1=std::any_cast<Input>(value);
            switch (di1) {
                case Input::ON:di1Item->setText(1,"ВКЛ");break;
                case Input::OFF:di1Item->setText(1,"ВЫКЛ");break;
                case Input::BREAK:di1Item->setText(1,"ОБРЫВ!");break;
                case Input::SHORT:di1Item->setText(1,"КЗ");break;
            }
        }else if(param=="di2") {
            di2=std::any_cast<Input>(value);
            switch (di2) {
                case Input::ON:di2Item->setText(1,"ВКЛ");break;
                case Input::OFF:di2Item->setText(1,"ВЫКЛ");break;
                case Input::BREAK:di2Item->setText(1,"ОБРЫВ!");break;
                case Input::SHORT:di2Item->setText(1,"КЗ");break;
            }
        }
        else if(param=="di3") {
            di3=std::any_cast<Input>(value);
            switch (di3) {
                case Input::ON:di3Item->setText(1,"ВКЛ");break;
                case Input::OFF:di3Item->setText(1,"ВЫКЛ");break;
                case Input::BREAK:di3Item->setText(1,"ОБРЫВ!");break;
                case Input::SHORT:di3Item->setText(1,"КЗ");break;
            }
        }
        else if(param=="do1") {
            do1=std::any_cast<bool>(value);
            if(do1) do1Item->setText(1,"Вкл");
            else do1Item->setText(1,"Выкл");
        }
        else if(param=="do2") {
            do2=std::any_cast<bool>(value);
            do2=std::any_cast<bool>(value);
            if(do2) do2Item->setText(1,"Вкл");
            else do2Item->setText(1,"Выкл");
        }
        else if(param=="real_point_cnt") {
            real_point_cnt=std::any_cast<int>(value);
            cntItem->setText(1,QString::number(real_point_cnt));
        }
        else if(param=="not_actual") {
            not_actual=std::any_cast<bool>(value);
            if(not_actual) {
                cntItem->setText(1,"нет данных");
                di1Item->setText(1,"нет данных");
                di2Item->setText(1,"нет данных");
                di3Item->setText(1,"нет данных");
                do1Item->setText(1,"нет данных");
                do2Item->setText(1,"нет данных");
            }
        }
    }
}

std::optional<std::any> Group::getGroupValue(const QString &param)
{
    if(param=="name") {return std::make_optional<std::any>(name);}
    else if(param=="real_point_cnt") {return std::make_optional<std::any>(real_point_cnt);}
    else if(param=="di1") {return std::make_optional<std::any>(di1);}
    else if(param=="di2") {return std::make_optional<std::any>(di2);}
    else if(param=="di3") {return std::make_optional<std::any>(di3);}
    else if(param=="do1") {return std::make_optional<std::any>(do1);}
    else if(param=="do2") {return std::make_optional<std::any>(do2);}
    return std::nullopt;
}
