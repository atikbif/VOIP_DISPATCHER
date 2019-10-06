#include "audiotree.h"
#include <exception>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

AudioTree::AudioTree(QTreeWidget *tree):tree(tree)
{

}

void AudioTree::setPointValue(int groupNum, int pointNum, const QString &param, std::any value)
{
    if(groupNum>=0 && groupNum<static_cast<int>(groups.size())) {
        groups[static_cast<std::vector<Group>::size_type>(groupNum)].setPointValue(pointNum,param,value);
    }
}

void AudioTree::setGroupValue(int groupNum, const QString &param, std::any value)
{
    if(groupNum>=0 && (groupNum < static_cast<int>(groups.size()))) {
        groups[static_cast<std::vector<Group>::size_type>(groupNum)].setGroupValue(param,value);
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

        QFile confFile("conf.json");
        if(confFile.open(QIODevice::ReadOnly)) {
            QByteArray saveData = confFile.readAll();
            QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
            QJsonObject loadOb = loadDoc.object();
            int groupCnt = 0;
            if(loadOb.contains("gate cnt")) {
              QString cnt = loadOb["gate cnt"].toString();
              groupCnt = cnt.toInt();
            }
            if(loadOb.contains("gates") && loadOb["gates"].isArray()) {
              QJsonArray jsGates = loadOb["gates"].toArray();
              int gate_length = jsGates.size();
              if(groupCnt>gate_length) groupCnt=gate_length;

              for (int i = 0; i < groupCnt; ++i) {
                QJsonObject gateOb = jsGates[i].toObject();
                if(gateOb.contains("name")) {
                  QString name = gateOb["name"].toString();
                  QTreeWidgetItem *gr = new QTreeWidgetItem();
                  tree->addTopLevelItem(gr);
                  Group g(gr,name);
                  int pointCnt = 0;
                  if(gateOb.contains("cnt")) {
                      pointCnt = gateOb["cnt"].toInt();
                  }
                  if(gateOb.contains("points")&& gateOb["points"].isArray()) {
                      QJsonArray jsPoints = gateOb["points"].toArray();
                      int points_length = jsPoints.size();
                      if(pointCnt>points_length) pointCnt=points_length;
                      for(int j=0;j<pointCnt;j++) {
                          QJsonObject pointOb = jsPoints[j].toObject();
                          if(pointOb.contains("name")) {
                              QString name = pointOb["name"].toString();
                              g.addNewPoint(name);
                          }
                      }
                  }
                  groups.push_back(g);
                }
              }
          }
      }
    }
}

