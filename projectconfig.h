#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QString>
#include <vector>

struct GateState{
    QString name;
    std::vector<QString> points;
    //int count;  // для совместимости со старым вариантом где количество задавалось отдельным параметром
    static const int maxPointQuantity;
};

class ProjectConfig
{
    QString fName;
public:
    static const int maxGateQuantity;
    QString ip1,ip2,ip3,ip4,tmr;
    std::vector<GateState> gates;
    explicit ProjectConfig(const QString &fileName);
    bool readConfig();
    bool writeConfig();
};

#endif // PROJECTCONFIG_H
