#ifndef POINTDATA_H
#define POINTDATA_H

#include <QByteArray>
#include "enums.h"
#include <QString>

class PointData
{
    unsigned char grNum;
    unsigned char pointNum;
    Input inp1,inp2;
    Inp2Type inp2type;
    bool out1, out2;
    double accVolt;
    double powVolt;
    Speaker dynamic;
    unsigned char soundReduction;
    unsigned char version;
    bool lastPointSwitch;
    double di1Filter;
    double di2Filter;
    static const unsigned char rawDataSize;
public:
    PointData(const QByteArray &rawPointData);
    bool getOutput1() const {return out1;}
    QString getOutput1String() const;
    bool getOutput2() const {return out2;}
    QString getOutput2String() const;
    Input getInput1() const {return inp1;}
    QString getInput1String() const;
    Input getInput2() const {return inp2;}
    QString getInput2String() const;
    double getAccumulatorVoltage() const {return accVolt;}
    double getPowerVoltage() const {return powVolt;}
    Speaker getSpeaker() const {return dynamic;}
    QString getSpeakerString() const;
    unsigned char getSoundReduction() const {return soundReduction;}
    unsigned char getVersion() const {return version;}
    bool getLastPointSwitch() const {return lastPointSwitch;}
    double getDI1Filter() const {return di1Filter;}
    double getDI2Filter() const {return di2Filter;}
    Inp2Type getInput2Type() const {return inp2type;}
    unsigned char getGroupNum() const {return grNum;}
    unsigned char getPointNum() const {return pointNum;}
    static unsigned char getRawDataSize() {return rawDataSize;}
};

#endif // POINTDATA_H
