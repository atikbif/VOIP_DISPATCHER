#ifndef GROUPDATA_H
#define GROUPDATA_H

#include "enums.h"
#include <QByteArray>
#include <QString>

class GroupData
{
    unsigned char pointsQuantity;
    Input di1, di2, di3;
    bool do1, do2;
    bool notActual;
    unsigned char grNum;
    unsigned char version;
    static const unsigned char rawGroupDataSize;
public:
    GroupData(const QByteArray &rawGroupData);
    Input getInput1() const {return di1;}
    QString getInput1String() const;
    Input getInput2() const {return di2;}
    QString getInput2String() const;
    Input getInput3() const {return di3;}
    QString getInput3String() const;
    unsigned char getPointsQuantity() const {return pointsQuantity;}
    bool getOut1() const {return do1;}
    QString getOut1String() const;
    bool getOut2() const {return do2;}
    QString getOut2String() const;
    unsigned char getGrNum() const {return grNum;}
    unsigned char getVersion() const {return version;}
    bool getNotActual() const {return notActual;}
    static unsigned char getRawGroupDataSize() {return rawGroupDataSize;}
};

#endif // GROUPDATA_H
