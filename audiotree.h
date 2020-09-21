#ifndef AUDIOTREE_H
#define AUDIOTREE_H

#include <QString>
#include <any>
#include <optional>
#include <QTreeWidget>
#include <vector>
#include "group.h"
#include "point.h"
#include "projectconfig.h"


class AudioTree
{
    std::vector<Group> groups;
    QTreeWidget *tree=nullptr;
    ProjectConfig *prConf;
public:
    explicit AudioTree(QTreeWidget *tree, ProjectConfig *prConf);
    void setPointValue(int groupNum, int pointNum, const QString &param, std::any value);
    void setGroupValue(int groupNum, const QString &param, std::any value);
    void setPointToDefault(int groupNum, int pointNum);
    std::optional<std::any> getPointValue(int groupNum, int pointNum, const QString &param);
    std::optional<std::any> getGroupValue(int groupNum, const QString &param);
    int groupCount();
    int pointCount(int groupNum);
    void createTree();
};

#endif // AUDIOTREE_H
