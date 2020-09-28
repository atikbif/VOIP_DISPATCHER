#include "pointdata.h"

const unsigned char PointData::rawDataSize = 9;

PointData::PointData(const QByteArray &rawPointData)
{
    if(rawPointData.size()>=rawDataSize) {
        grNum = static_cast<quint8>(rawPointData.at(0));
        pointNum = static_cast<quint8>(rawPointData.at(1));
        accVolt = static_cast<quint8>(rawPointData.at(2))*0.1;
        powVolt = static_cast<quint8>(rawPointData.at(3))*0.1;
        version = static_cast<quint8>(rawPointData.at(4));
        quint16 bits = static_cast<quint8>(rawPointData.at(5));
        bits = static_cast<quint16>((static_cast<quint16>(bits)<<8) | static_cast<quint8>(rawPointData.at(6)));
        soundReduction = static_cast<quint8>(rawPointData.at(7));
        quint8 diFilters = static_cast<quint8>(rawPointData.at(8));
        di1Filter = (diFilters & 0x0F)*0.5;
        di2Filter = (diFilters >> 4)*0.5;
        bool di1_state = bits & (1<<0);
        bool di1_break = bits & (1<<1);
        bool di1_short = bits & (1<<2);
        bool di1_off = bits & (1<<11);
        if(di1_off) inp1 = Input::UNUSED;
        else {
            if(di1_break) inp1 = Input::BREAK;
            else if(di1_short) inp1 = Input::SHORT;
            else if(di1_state) inp1 = Input::ON;
            else inp1 = Input::OFF;
        }

        bool di2_state = bits & (1<<3);
        bool di2_break = bits & (1<<4);
        bool di2_short = bits & (1<<5);
        bool di2_off = bits & (1<<12);
        if(di2_off) inp2 = Input::UNUSED;
        else {
            if(di2_break) inp2 = Input::BREAK;
            else if(di2_short) inp2 = Input::SHORT;
            else if(di2_state) inp2 = Input::ON;
            else inp2 = Input::OFF;
        }

        out1 = bits & (1<<6);
        out2 = bits & (1<<7);

        bool speaker_state = bits & (1<<8);
        bool speaker_check = bits & (1<<9);
        if(speaker_check==false) dynamic = Speaker::NOT_CHECKED;
        else {
            if(speaker_state) dynamic = Speaker::CORRECT;
            else dynamic = Speaker::PROBLEM;
        }
        lastPointSwitch = bits & (1<<10);

        quint8 di2Type = 0;
        if(bits & (1<<13)) di2Type |= 0x01;
        if(bits & (1<<14)) di2Type |= 0x02;
        switch(di2Type) {
            case 0:inp2type = Inp2Type::KSL;break;
            case 1:inp2type = Inp2Type::FENCE;break;
            case 2:inp2type = Inp2Type::JAMMING;break;
            case 3:inp2type = Inp2Type::CROSSING;break;
        }
    }
}

QString PointData::getOutput1String() const
{
    QString res="-";
    if(out1) res="вкл";
    else res="выкл";
    return res;
}

QString PointData::getOutput2String() const
{
    QString res="-";
    if(out2) res="вкл";
    else res="выкл";
    return res;
}

QString PointData::getInput1String() const
{
    QString res="-";
    if(inp1==Input::UNUSED) res = "не используется";
    else if(inp1==Input::BREAK) res = "обрыв";
    else if(inp1==Input::SHORT) res = "замыкание";
    else if(inp1==Input::ON) res = "вкл";
    else if(inp1==Input::OFF) res = "выкл";
    return res;
}

QString PointData::getInput2String() const
{
    QString res="-";
    if(inp2==Input::UNUSED) res = "не используется";
    else if(inp2==Input::BREAK) res = "обрыв";
    else if(inp2==Input::SHORT) res = "замыкание";
    else if(inp2==Input::ON) res = "вкл";
    else if(inp2==Input::OFF) res = "выкл";
    return res;
}

QString PointData::getSpeakerString() const
{
    QString res="-";
    if(dynamic == Speaker::NOT_CHECKED) res = "не проверялись";
    else if(dynamic == Speaker::CORRECT) res = "исправны";
    else if(dynamic == Speaker::PROBLEM) res = "не исправны";
    return res;
}
