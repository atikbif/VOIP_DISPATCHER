#include "projectconfig.h"
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const int GateState::maxPointQuantity = 100;
const int ProjectConfig::maxGateQuantity = 32;

ProjectConfig::ProjectConfig(const QString &fileName): fName(fileName)
{
    readConfig();
}

bool ProjectConfig::readConfig()
{
    bool readConf = false;
    QFile confFile(fName);
    gates.clear();
    int gateCnt = 0;    // для совместимости с прошлыми версиями где количество задавалось отдельным параметром
    if(confFile.open(QIODevice::ReadOnly)) {
        QByteArray saveData = confFile.readAll();
        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject loadOb = loadDoc.object();
        if(loadOb.contains("ip1")) {
            ip1 = loadOb["ip1"].toString();
        }
        if(loadOb.contains("ip2")) {
            ip2 = loadOb["ip2"].toString();
        }
        if(loadOb.contains("ip3")) {
            ip3 = loadOb["ip3"].toString();
        }
        if(loadOb.contains("ip4")) {
            ip4 = loadOb["ip4"].toString();
        }
        if(loadOb.contains("audio tmr")) {
            tmr = loadOb["audio tmr"].toString();
        }
        bool gateCntFlag = false;
        if(loadOb.contains("gate cnt")) {
            QString gateCntStr = loadOb["gate cnt"].toString();
            gateCnt = gateCntStr.toInt();
            if(gateCnt>maxGateQuantity) gateCnt = maxGateQuantity;
            gateCntFlag = true;
        }
        if(loadOb.contains("gates") && loadOb["gates"].isArray()) {
            QJsonArray jsGates = loadOb["gates"].toArray();
            int gateRealCnt = jsGates.size();
            if(gateRealCnt>maxGateQuantity) gateRealCnt=maxGateQuantity;
            int gateLimit = gateRealCnt;
            if(gateCntFlag) gateLimit = std::min(gateCnt,gateRealCnt);

            for (int i = 0; i < gateLimit; ++i) {
                GateState gate;
                QJsonObject gateOb = jsGates[i].toObject();
                if(gateOb.contains("name")) {
                    QString name = gateOb["name"].toString();
                    gate.name = name;
                }
                int pointCnt = 0;    // для совместимости с прошлыми версиями где количество задавалось отдельным параметром
                bool pointCntFlag = false;
                if(gateOb.contains("cnt")) {
                    pointCnt = gateOb["cnt"].toInt();
                    if(pointCnt>GateState::maxPointQuantity) pointCnt=GateState::maxPointQuantity;
                    pointCntFlag = true;
                }
                if(gateOb.contains("points")&& gateOb["points"].isArray()) {
                    QJsonArray jsPoints = gateOb["points"].toArray();
                    int pointRealCnt = jsPoints.size();
                    if(pointRealCnt>GateState::maxPointQuantity) pointRealCnt=GateState::maxPointQuantity;
                    int pointLimit = pointRealCnt;
                    if(pointCntFlag) pointLimit = std::min(pointCnt,pointRealCnt);
                    for(int j=0;j<pointLimit;j++) {
                        QJsonObject pointOb = jsPoints[j].toObject();
                        if(pointOb.contains("name")) {
                            QString name = pointOb["name"].toString();
                            gate.points.push_back(name);
                            readConf = true;
                        }
                    }
                }
                gates.push_back(gate);
            }
        }
    }
    if(readConf==false) {
        GateState gate;
        gate.name = "ГРУППА 1";
        for(int i=0;i<2;i++) {
            gate.points.push_back("Точка "+QString::number(i+1));
        }
        gates.push_back(gate);

        gate.points.clear();
        gate.name = "ГРУППА 2";
        for(int i=0;i<3;i++) {
            gate.points.push_back("Точка "+QString::number(i+1));
        }
        gates.push_back(gate);

        ip1="192";
        ip2="168";
        ip3="1";
        ip4="2";
    }
    return readConf;
}

bool ProjectConfig::writeConfig()
{
    QFile confFile(fName);
    if(confFile.open(QIODevice::WriteOnly)) {
        QJsonObject confObject;
        QJsonArray gateArray;

        for(const GateState &gate:gates) {
            QJsonObject g;
            g["name"] = gate.name;
            //g["cnt"] = gate.count;
            QJsonArray pointArray;
            for(const QString &pname:gate.points) {
              QJsonObject p;
              p["name"]=pname;
              pointArray.append(p);
            }
            g["points"] = pointArray;
            gateArray.append(g);
        }

        confObject["version"] = "1.1";
        //confObject["gate cnt"] = QString::number(gates.size());
        confObject["audio tmr"] = tmr;
        confObject["gates"] = gateArray;
        confObject["ip1"] = ip1;
        confObject["ip2"] = ip2;
        confObject["ip3"] = ip3;
        confObject["ip4"] = ip4;
        QJsonDocument confDoc(confObject);
        confFile.write(confDoc.toJson());
        confFile.close();
        return true;
    }
    return false;
}
