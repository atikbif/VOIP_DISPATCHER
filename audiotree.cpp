#include "audiotree.h"
#include <exception>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

AudioTree::AudioTree(QTreeWidget *tree, ProjectConfig *prConf):tree(tree), prConf(prConf)
{

}

void AudioTree::setPointValue(int groupNum, int pointNum, const QString &param, std::any value)
{
    if(groupNum>=0 && groupNum<static_cast<int>(groups.size())) {
        auto cnt = getGroupValue(groupNum,"real_point_cnt");
        if(cnt) {
            int real_cnt = std::any_cast<int>(cnt.value());
            if(pointNum<real_cnt) groups[static_cast<std::vector<Group>::size_type>(groupNum)].setPointValue(pointNum,param,value);
        }

    }
}

void AudioTree::setGroupValue(int groupNum, const QString &param, std::any value)
{
    if(groupNum>=0 && (groupNum < static_cast<int>(groups.size()))) {
        groups[static_cast<std::vector<Group>::size_type>(groupNum)].setGroupValue(param,value);
    }
}

void AudioTree::setPointToDefault(int groupNum, int pointNum)
{
    if(groupNum>=0 && (groupNum < static_cast<int>(groups.size()))) {
        groups[static_cast<std::vector<Group>::size_type>(groupNum)].setPointToDefault(pointNum);
    }
}

std::optional<std::any> AudioTree::getPointValue(int groupNum, int pointNum, const QString &param)
{
    if(groupNum>=0 && groupNum<static_cast<int>(groups.size())) {
        return groups[static_cast<std::vector<Group>::size_type>(groupNum)].getPointValue(pointNum,param);
    }
    return std::nullopt;
}

std::optional<std::any> AudioTree::getGroupValue(int groupNum, const QString &param)
{
    if(groupNum>=0 && groupNum<static_cast<int>(groups.size())) {
        return groups[static_cast<std::vector<Group>::size_type>(groupNum)].getGroupValue(param);
    }
    return std::nullopt;
}

int AudioTree::groupCount()
{
    return static_cast<int>(groups.size());
}

int AudioTree::pointCount(int groupNum)
{
    if(groupNum>=0 && groupNum<static_cast<int>(groups.size())) {
        return groups[static_cast<std::vector<Group>::size_type>(groupNum)].count();
    }
    return 0;
}

void AudioTree::createTree()
{
    if(tree) {
        prConf->readConfig();
        int groupCnt = static_cast<int>(prConf->gates.size());
        for (int i = 0; i < groupCnt; ++i) {
            QString gr_name = prConf->gates.at(i).name;
            QTreeWidgetItem *gr = new QTreeWidgetItem();
            tree->addTopLevelItem(gr);
            Group g(gr,gr_name);
            int pCnt = static_cast<int>(prConf->gates.at(i).points.size());
            for(int j=0;j<pCnt;j++) {
                g.addNewPoint(prConf->gates.at(i).points.at(j));
            }
            groups.push_back(g);
        }
    }
}

