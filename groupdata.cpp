#include "groupdata.h"

const unsigned char GroupData::rawGroupDataSize = 5;

GroupData::GroupData(const QByteArray &rawGroupData)
{
    grNum = static_cast<unsigned char>(rawGroupData.at(0));
    pointsQuantity = static_cast<unsigned char>(rawGroupData.at(1));
    version = static_cast<unsigned char>(rawGroupData.at(2));
    unsigned short bits = static_cast<unsigned char>(rawGroupData.at(3));
    bits = (bits<<8) | static_cast<unsigned char>(rawGroupData.at(4));
    bool di1_state = bits & (1<<0);
    bool di1_break = bits & (1<<1);
    bool di1_short = bits & (1<<2);
    if(di1_break) di1 = Input::BREAK;
    else if(di1_short) di1 = Input::SHORT;
    else if(di1_state) di1 = Input::ON;
    else di1 = Input::OFF;
    bool di2_state = bits & (1<<3);
    bool di2_break = bits & (1<<4);
    bool di2_short = bits & (1<<5);
    if(di2_break) di2 = Input::BREAK;
    else if(di2_short) di2 = Input::SHORT;
    else if(di2_state) di2 = Input::ON;
    else di2 = Input::OFF;
    bool di3_state = bits & (1<<6);
    bool di3_break = bits & (1<<7);
    bool di3_short = bits & (1<<8);
    if(di3_break) di3 = Input::BREAK;
    else if(di3_short) di3 = Input::SHORT;
    else if(di3_state) di3 = Input::ON;
    else di3 = Input::OFF;
    do1 = bits & (1<<9);
    do2 = bits & (1<<10);
    notActual = bits & (1<<13);
}

QString GroupData::getInput1String() const
{
    QString res="-";
    if(di1==Input::BREAK) res = "обрыв";
    else if(di1==Input::SHORT) res = "замыкание";
    else if(di1==Input::ON) res = "вкл";
    else if(di1==Input::OFF) res = "выкл";
    return res;
}

QString GroupData::getInput2String() const
{
    QString res="-";
    if(di2==Input::BREAK) res = "обрыв";
    else if(di2==Input::SHORT) res = "замыкание";
    else if(di2==Input::ON) res = "вкл";
    else if(di2==Input::OFF) res = "выкл";
    return res;
}

QString GroupData::getInput3String() const
{
    QString res="-";
    if(di3==Input::BREAK) res = "обрыв";
    else if(di3==Input::SHORT) res = "замыкание";
    else if(di3==Input::ON) res = "вкл";
    else if(di3==Input::OFF) res = "выкл";
    return res;
}

QString GroupData::getOut1String() const
{
    QString res="-";
    if(do1) res="вкл"; else res="выкл";
    return res;
}

QString GroupData::getOut2String() const
{
    QString res="-";
    if(do2) res="вкл"; else res="выкл";
    return res;
}
