#include "sqldriver.h"
#include <QThread>
#include "QSqlQuery"
#include <QSqlRecord>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

void SQLDriver::initDataBase()
{
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    //db.setDatabaseName("Driver={MySQL ODBC 8.0 Unicode Driver};Database=voip;");
    db.setDatabaseName("Driver={PostgreSQL Unicode};Database=voip;");
    db.setHostName("localhost");
    db.setUserName("postgres");
    db.setPassword("interrupt");
    db.setPort(5432);
    if (!db.open())
    {
        emit error("ОШИБКА ПОДКЛЮЧЕНИЯ К БАЗЕ ДАННЫХ!");
    }else {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS points ("
                   "id SERIAL PRIMARY KEY NOT NULL,"
                   "num smallint NOT NULL,"
                   "gate smallint NOT NULL,"
                   "di1 TEXT NOT NULL, "
                   "di2 TEXT NOT NULL, "
                   "do1 TEXT NOT NULL, "
                   "do2 TEXT NOT NULL, "
                   "speaker TEXT NOT NULL, "
                   "pow REAL NOT NULL,"
                   "bat REAL NOT NULL,"
                   "ip TEXT NOT NULL,"
                   "tmr TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL"
                   ");");
        query.exec("CREATE TABLE IF NOT EXISTS gates ("
                   "id SERIAL PRIMARY KEY NOT NULL,"
                   "num INTEGER NOT NULL,"
                   "ip TEXT NOT NULL,"
                   "cnt smallint NOT NULL,"
                   "di1 TEXT NOT NULL, "
                   "di2 TEXT NOT NULL, "
                   "di3 TEXT NOT NULL, "
                   "do1 TEXT NOT NULL, "
                   "do2 TEXT NOT NULL, "
                   "tmr TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL"
                   ");");
        query.exec("CREATE TABLE IF NOT EXISTS point_alarms ("
                   "id SERIAL PRIMARY KEY NOT NULL,"
                   "alarm TEXT,"
                   "type TEXT,"
                   "point smallint NOT NULL,"
                   "gate smallint NOT NULL,"
                   "ip TEXT NOT NULL,"
                   "tmr TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL"
                   ");");
        query.exec("CREATE TABLE IF NOT EXISTS gate_alarms ("
                   "id SERIAL PRIMARY KEY NOT NULL,"
                   "alarm TEXT,"
                   "type TEXT,"
                   "gate smallint NOT NULL,"
                   "ip TEXT NOT NULL,"
                   "tmr TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL"
                   ");");
        query.exec("CREATE TABLE IF NOT EXISTS journal ("
                   "id SERIAL PRIMARY KEY NOT NULL,"
                   "message VARCHAR(64),"
                   "type TEXT,"
                   "tmr TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL"
                   ");");
    }
    journalModel = new ColoredSqlQueryModel(nullptr);
    pointModel = new ColoredSqlQueryModel(nullptr);
    groupModel = new ColoredSqlQueryModel(nullptr);
    groupAlarmModel = new ColoredSqlQueryModel(nullptr);
    pointAlarmModel = new ColoredSqlQueryModel(nullptr);
}

void SQLDriver::insertDatatoDataBase()
{
    double acc, pow;
    QString di1,di2,do1,do2,speaker;
    QString gdi1,gdi2,gdi3,gdo1,gdo2;


    static qint64 last_sec = 0;
    static bool next_insert = false;
    static int offset = 0;

    if(QDateTime::currentSecsSinceEpoch()-last_sec>=10 || next_insert) {
        last_sec = QDateTime::currentSecsSinceEpoch();

        quint16 start_pos = 0;
        quint16 end_pos = static_cast<quint16>(static_cast<quint8>(rawData.at(1)))<<8 | static_cast<quint8>(rawData.at(2));
        if(next_insert) {
            start_pos = offset;
        }
        if(end_pos-start_pos>30) {
            end_pos = start_pos+30;
            offset = end_pos;
            next_insert = true;
        }else next_insert = false;
        for(quint16 i=start_pos;i<end_pos;i++) {
            quint16 reg_offset = 3+i*8;
            if(reg_offset+7>=rawData.length()) return;
            quint8 gr_num = static_cast<quint8>(rawData.at(reg_offset));
            quint8 point_num = static_cast<quint8>(rawData.at(reg_offset+1));
            if(gr_num && groupCorrectDataFlag.at(gr_num-1)) {
                acc = static_cast<double>(static_cast<quint8>(rawData.at(reg_offset+2)))/10;
                pow = static_cast<double>(static_cast<quint8>(rawData.at(reg_offset+3)))/10;

                quint16 bits = static_cast<quint8>(rawData.at(reg_offset+5));
                bits = (bits<<8) | static_cast<quint8>(rawData.at(reg_offset+6));

                di1="выкл";
                if(bits & (1<<0)) di1 = "вкл";
                else if(bits & (1<<1)) {di1 = "обрыв";}
                else if(bits & (1<<2)) {di1 = "замыкание";}
                di2="выкл";
                if(bits & (1<<3)) di2 = "вкл";
                else if(bits & (1<<4)) {di2 = "обрыв";}
                else if(bits & (1<<5)) {di2 = "замыкание";}
                if(bits & (1<<6)) do1="вкл";else do1="выкл";
                if(bits & (1<<7)) do2="вкл";else do2="выкл";
                speaker = "не исправны";
                if(bits & (1<<8)) {speaker = "исправны";}
                if((bits & (1<<9))==0) {speaker = "не проверялись";}
            }else {
                di1="нет данных";
                di2="нет данных";
                do1="нет данных";
                do2="нет данных";
                acc = 0;
                pow = 0;
                speaker = "нет данных";
                addPointAlarm(ip,point_num,gr_num,"нет данных", "авария");
            }

            QSqlQuery query;
            query.prepare("INSERT INTO points (num, gate, di1, di2, do1, do2, speaker, pow, bat, ip)"
                                                  "VALUES (:num, :gate, :di1, :di2, :do1, :do2, :speaker, :pow, :bat, :ip);");
            query.bindValue(":num", point_num);
            query.bindValue(":gate", gr_num);
            query.bindValue(":di1", di1);
            query.bindValue(":di2", di2);
            query.bindValue(":do1", do1);
            query.bindValue(":do2", do2);
            query.bindValue(":speaker", speaker);
            query.bindValue(":pow", pow);
            query.bindValue(":bat", acc);
            query.bindValue(":ip", ip);
            query.exec();


            if(di1=="вкл" && speaker=="исправны") addPointAlarm(ip,point_num,gr_num,"параметры в норме", "сообщение");
            else {
                if(di1=="обрыв") addPointAlarm(ip,point_num,gr_num,"Обрыв DI1", "авария");
                if(di1=="замыкание") addPointAlarm(ip,point_num,gr_num,"Замыкание DI1", "авария");
                if(di1=="выкл") addPointAlarm(ip,point_num,gr_num,"DI1 выкл", "авария");
                if(speaker=="не проверялись") addPointAlarm(ip,point_num,gr_num,"Динамики не проверялись", "предупреждение");
                if(speaker=="не исправны") addPointAlarm(ip,point_num,gr_num,"Динамики не исправны", "авария");
            }
        }
    }else {
        quint16 cnt = static_cast<quint16>(static_cast<quint8>(rawData.at(1)))<<8 | static_cast<quint8>(rawData.at(2));
        quint16 last_cnt = static_cast<quint16>(static_cast<quint8>(lastData.at(1)))<<8 | static_cast<quint8>(lastData.at(2));
        if(cnt==last_cnt) for(quint16 i=0;i<cnt;i++) {
            quint16 reg_offset = 3+i*8;
            if(reg_offset+7>=rawData.length()) return;
            quint8 gr_num = static_cast<quint8>(rawData.at(reg_offset));
            quint8 point_num = static_cast<quint8>(rawData.at(reg_offset+1));
            acc = static_cast<double>(static_cast<quint8>(rawData.at(reg_offset+2)))/10;
            pow = static_cast<double>(static_cast<quint8>(rawData.at(reg_offset+3)))/10;

            quint16 bits = static_cast<quint8>(rawData.at(reg_offset+5));
            bits = (bits<<8) | static_cast<quint8>(rawData.at(reg_offset+6));

            di1="выкл";
            if(bits & (1<<0)) di1 = "вкл";
            else if(bits & (1<<1)) {di1 = "обрыв";}
            else if(bits & (1<<2)) {di1 = "замыкание";}
            di2="выкл";
            if(bits & (1<<3)) di2 = "вкл";
            else if(bits & (1<<4)) {di2 = "обрыв";}
            else if(bits & (1<<5)) {di2 = "замыкание";}
            if(bits & (1<<6)) do1="вкл";else do1="выкл";
            if(bits & (1<<7)) do2="вкл";else do2="выкл";
            speaker = "не исправны";
            if(bits & (1<<8)) {speaker = "исправны";}
            if((bits & (1<<9))==0) {speaker = "не проверялись";}

            quint8 last_gr_num = static_cast<quint8>(lastData.at(reg_offset));
            quint8 last_point_num = static_cast<quint8>(lastData.at(reg_offset+1));

            if(last_gr_num==gr_num && last_point_num==point_num) {
                double last_acc, last_pow;
                QString last_di1, last_di2, last_do1, last_do2,last_speaker;
                QString last_gdi1, last_gdi2, last_gdi3, last_gdo1, last_gdo2;

                if(gr_num && groupCorrectDataFlag.at(gr_num-1)) {
                    last_acc = static_cast<double>(static_cast<quint8>(lastData.at(reg_offset+2)))/10;
                    last_pow = static_cast<double>(static_cast<quint8>(lastData.at(reg_offset+3)))/10;

                    quint16 last_bits = static_cast<quint8>(lastData.at(reg_offset+5));
                    last_bits = (last_bits<<8) | static_cast<quint8>(lastData.at(reg_offset+6));

                    last_di1="выкл";
                    if(last_bits & (1<<0)) last_di1 = "вкл";
                    else if(last_bits & (1<<1)) {last_di1 = "обрыв";}
                    else if(last_bits & (1<<2)) {last_di1 = "замыкание";}
                    last_di2="выкл";
                    if(last_bits & (1<<3)) last_di2 = "вкл";
                    else if(last_bits & (1<<4)) {last_di2 = "обрыв";}
                    else if(last_bits & (1<<5)) {last_di2 = "замыкание";}
                    if(last_bits & (1<<6)) last_do1="вкл";else last_do1="выкл";
                    if(last_bits & (1<<7)) last_do2="вкл";else last_do2="выкл";
                    last_speaker = "не исправны";
                    if(last_bits & (1<<8)) {last_speaker = "исправны";}
                    if((last_bits & (1<<9))==0) {last_speaker = "не проверялись";}
                }else {
                    di1="нет данных";
                    di2="нет данных";
                    do1="нет данных";
                    do2="нет данных";
                    acc=0;
                    pow=0;
                    speaker = "нет данных";
                    last_di1="нет данных";
                    last_di2="нет данных";
                    last_do1="нет данных";
                    last_do2="нет данных";
                    last_speaker = "нет данных";
                    last_acc=0;
                    last_pow=0;
                }


                if((last_acc!=acc)||(last_pow!=pow)||(di1!=last_di1)||(di2!=last_di2)||(do1!=last_do1)||(do2!=last_do2)||(speaker!=last_speaker)) {
                    QSqlQuery query;
                    query.prepare("INSERT INTO points (num, gate, di1, di2, do1, do2, speaker, pow, bat, ip)"
                                                          "VALUES (:num, :gate, :di1, :di2, :do1, :do2, :speaker, :pow, :bat, :ip);");
                    query.bindValue(":num", point_num);
                    query.bindValue(":gate", gr_num);
                    query.bindValue(":di1", di1);
                    query.bindValue(":di2", di2);
                    query.bindValue(":do1", do1);
                    query.bindValue(":do2", do2);
                    query.bindValue(":speaker", speaker);
                    query.bindValue(":pow", pow);
                    query.bindValue(":bat", acc);
                    query.bindValue(":ip", ip);

                    query.exec();


                    if(last_di1!=di1) {
                        if(di1=="обрыв") addPointAlarm(ip,point_num,gr_num,"Обрыв DI1", "авария");
                        if(di1=="замыкание") addPointAlarm(ip,point_num,gr_num,"Замыкание DI1", "авария");
                        if(di1=="выкл") addPointAlarm(ip,point_num,gr_num,"DI1 выкл", "авария");
                        if(di1=="вкл") addPointAlarm(ip,point_num,gr_num,"DI1 вкл", "сообщение");
                    }
                    if(last_speaker!=speaker) {
                        if(speaker=="не проверялись") addPointAlarm(ip,point_num,gr_num,"Динамики не проверялись", "предупреждение");
                        if(speaker=="не исправны") addPointAlarm(ip,point_num,gr_num,"Динамики не исправны", "авария");
                        if(speaker=="исправны") addPointAlarm(ip,point_num,gr_num,"Динамики исправны", "сообщение");
                    }
                }
            }

        }else {
            next_insert = true;
            offset = 0;
        }
    }
    lastData.clear();
    lastData.append(rawData);
}

void SQLDriver::insertGroupDatatoDataBase()
{
    QString gdi1,gdi2,gdi3,gdo1,gdo2;
    bool gnot_act;
    QString last_gdi1, last_gdi2, last_gdi3, last_gdo1, last_gdo2;
    static qint64 last_sec = 0;

    if(QDateTime::currentSecsSinceEpoch()-last_sec>=10) {
        last_sec = QDateTime::currentSecsSinceEpoch();
        int length = rawGroupData.length()/5;
        for(int i=0;i<length;i++) {
            if(rawGroupData.at(i*5)==i+1) {
                uint16_t bits = (quint8)rawGroupData.at(i*5+3);
                bits = (((quint16)bits)<<8) | (quint8)rawGroupData.at(i*5+4);

                if(bits & (1<<11)) gnot_act = true;else gnot_act=false;
                uint8_t cnt = 0;

                if(gnot_act==false) {
                    groupCorrectDataFlag[i]=true;
                    cnt = (quint8)rawGroupData.at(i*5+1);
                    gdi1 = "выкл";
                    if(bits & (1<<0)) gdi1 = "вкл";
                    else if(bits & (1<<1)) gdi1 = "обрыв";
                    else if(bits & (1<<2)) gdi1 = "замыкание";
                    gdi2 = "выкл";
                    if(bits & (1<<3)) gdi2 = "вкл";
                    else if(bits & (1<<4)) gdi2 = "обрыв";
                    else if(bits & (1<<5)) gdi2 = "замыкание";
                    gdi3 = "выкл";
                    if(bits & (1<<6)) gdi3 = "вкл";
                    else if(bits & (1<<7)) gdi3 = "обрыв";
                    else if(bits & (1<<8)) gdi3 = "замыкание";
                    if(bits & (1<<9)) gdo1="вкл";else gdo1="выкл";
                    if(bits & (1<<10)) gdo2="вкл";else gdo2="выкл";
                }else {
                    groupCorrectDataFlag[i]=false;
                    cnt = 0;
                    gdi1 = "нет данных";
                    gdi2 = "нет данных";
                    gdi3 = "нет данных";
                    gdo1="нет данных";
                    gdo2="нет данных";
                }

                QSqlQuery query;
                query.prepare("INSERT INTO gates (num, ip, cnt, di1, di2, di3, do1, do2)"
                                                      "VALUES (:num, :ip, :cnt, :di1, :di2, :di3, :do1, :do2);");
                query.bindValue(":num", i+1);
                query.bindValue(":ip", ip);
                query.bindValue(":cnt", cnt);
                query.bindValue(":di1", gdi1);
                query.bindValue(":di2", gdi2);
                query.bindValue(":di3", gdi3);
                query.bindValue(":do1", gdo1);
                query.bindValue(":do2", gdo2);
                query.exec();


                if(gnot_act) {
                    addGateAlarm(ip,i+1,"нет данных", "авария");
                }else {
                    if(cnt>=point_cnt.at(i)) {
                        addGateAlarm(ip,i+1,"подключено " + QString::number(cnt) + " точек", "сообщение");
                    }else {
                        addGateAlarm(ip,i+1,"подключено " + QString::number(cnt) + " точек", "авария");
                    }
                }
            }
        }
    }else {
        int length = rawGroupData.length()/5;
        int lastLength = rawGroupData.length()/5;

        if(length==lastLength)
        for(int i=0;i<length;i++) {
            if(rawGroupData.at(i*5)==i+1) {
                bool last_gnot_act = false;
                uint16_t bits = (quint8)rawGroupData.at(i*5+3);
                bits = (bits<<8) | (quint8)rawGroupData.at(i*5+4);
                uint8_t cnt = (quint8)rawGroupData.at(i*5+1);
                uint16_t last_bits = (quint8)lastGroupData.at(i*5+3);
                last_bits = (last_bits<<8) | (quint8)lastGroupData.at(i*5+4);
                uint8_t last_cnt = (quint8)lastGroupData.at(i*5+1);
                if(bits & (1<<11)) gnot_act = true;else gnot_act=false;
                if(last_bits & (1<<11)) last_gnot_act = true;else last_gnot_act=false;

                if(gnot_act) {
                    groupCorrectDataFlag[i]=false;
                    gdi1 = "нет данных";
                    gdi2 = "нет данныхл";
                    gdi3 = "нет данных";
                    gdo1="выкл";
                    gdo2="выкл";
                    last_gdi1 = "нет данных";
                    last_gdi2 = "нет данных";
                    last_gdi3 = "нет данных";
                    last_gdo1="нет данных";
                    last_gdo2="нет данных";
                }else {
                    groupCorrectDataFlag[i]=true;
                    gdi1 = "выкл";
                    if(bits & (1<<0)) gdi1 = "вкл";
                    else if(bits & (1<<1)) gdi1 = "обрыв";
                    else if(bits & (1<<2)) gdi1 = "замыкание";
                    gdi2 = "выкл";
                    if(bits & (1<<3)) gdi2 = "вкл";
                    else if(bits & (1<<4)) gdi2 = "обрыв";
                    else if(bits & (1<<5)) gdi2 = "замыкание";
                    gdi3 = "выкл";
                    if(bits & (1<<6)) gdi3 = "вкл";
                    else if(bits & (1<<7)) gdi3 = "обрыв";
                    else if(bits & (1<<8)) gdi3 = "замыкание";
                    if(bits & (1<<9)) gdo1="вкл";else gdo1="выкл";
                    if(bits & (1<<10)) gdo2="вкл";else gdo2="выкл";
                    last_gdi1 = "выкл";
                    if(last_bits & (1<<0)) last_gdi1 = "вкл";
                    else if(last_bits & (1<<1)) last_gdi1 = "обрыв";
                    else if(last_bits & (1<<2)) last_gdi1 = "замыкание";
                    last_gdi2 = "выкл";
                    if(last_bits & (1<<3)) last_gdi2 = "вкл";
                    else if(last_bits & (1<<4)) last_gdi2 = "обрыв";
                    else if(last_bits & (1<<5)) last_gdi2 = "замыкание";
                    last_gdi3 = "выкл";
                    if(last_bits & (1<<6)) last_gdi3 = "вкл";
                    else if(last_bits & (1<<7)) last_gdi3 = "обрыв";
                    else if(last_bits & (1<<8)) last_gdi3 = "замыкание";
                    if(last_bits & (1<<9)) last_gdo1="вкл";else last_gdo1="выкл";
                    if(last_bits & (1<<10)) last_gdo2="вкл";else last_gdo2="выкл";
                }


                if((gdi1!=last_gdi1)||(gdi2!=last_gdi2)||(gdi3!=last_gdi3)||(gdo1!=last_gdo1)||(gdo2!=last_gdo2)||(cnt!=last_cnt)||(gnot_act!=last_gnot_act)) {
                    QSqlQuery query;
                    query.prepare("INSERT INTO gates (num, ip, cnt, di1, di2, di3, do1, do2)"
                                                          "VALUES (:num, :ip, :cnt, :di1, :di2, :di3, :do1, :do2);");
                    query.bindValue(":num", i+1);
                    query.bindValue(":ip", ip);
                    query.bindValue(":cnt", cnt);
                    query.bindValue(":di1", gdi1);
                    query.bindValue(":di2", gdi2);
                    query.bindValue(":di3", gdi3);
                    query.bindValue(":do1", gdo1);
                    query.bindValue(":do2", gdo2);
                    query.exec();
                }
                if(gnot_act==false) {
                    if(cnt!=last_cnt) {
                        if(cnt>=point_cnt.at(i)) {
                            addGateAlarm(ip,i+1,"подключено " + QString::number(cnt) + " точек", "сообщение");
                        }else {
                            addGateAlarm(ip,i+1,"подключено " + QString::number(cnt) + " точек", "авария");
                        }
                    }
                }

                if(gnot_act!=last_gnot_act) {
                    if(gnot_act) {
                        addGateAlarm(ip,i+1,"нет данных", "авария");
                    }else {
                        addGateAlarm(ip,i+1,"группа подключена", "сообщение");
                    }
                }
            }
        }
    }
    lastGroupData.clear();
    lastGroupData.append(rawGroupData);
}

SQLDriver::SQLDriver(QObject *parent) : QObject(parent)
{
    for(int i=0;i<32;i++) point_cnt.push_back(0);
    std::fill(groupCorrectDataFlag.begin(),groupCorrectDataFlag.end(),0);
}

void SQLDriver::finish()
{
    QMutexLocker locker(&mutex);
    finishFlag = true;
}

void SQLDriver::initDB()
{
    QMutexLocker locker(&mutex);
    initFlag = true;
}

void SQLDriver::insertData(const QByteArray &data)
{
    QMutexLocker locker(&mutex);
    rawData.clear();
    rawData.append(data);
    insertFlag = true;
}

void SQLDriver::insertGroupData(const QByteArray &data)
{
    QMutexLocker locker(&mutex);
    rawGroupData.clear();
    rawGroupData.append(data);
    insertGroupFlag = true;
}

void SQLDriver::insertMessage(const QString &text, const QString &type)
{
    QMutexLocker locker(&mutex);
    Message message;
    message.text = text;
    message.type = type;
    messages.push_back(message);
}

void SQLDriver::updateJournal(const QDate &from, const QDate &to, QTableView *tv)
{
    QMutexLocker locker(&mutex);

    journalQuery = "SELECT tmr,type,message FROM journal "
            "where tmr between '";
    journalQuery += from.toString(" yyyy-MM-dd 00:00:00");
    journalQuery += "' and '";
    journalQuery += to.toString(" yyyy-MM-dd 23:59:59");
    journalQuery += "' ORDER BY id DESC;";
    tv->setModel(journalModel);
}

void SQLDriver::updatePointArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point)
{
    QMutexLocker locker(&mutex);

    pointQuery = "SELECT tmr,di1,di2,do1,do2,pow,bat,speaker,ip FROM points "
            "where tmr between '";
    pointQuery += from.toString("yyyy-MM-dd 00:00:00");
    pointQuery += "' and '";
    pointQuery += to.toString("yyyy-MM-dd 23:59:59");
    pointQuery += "' and gate="+QString::number(gr);
    pointQuery += " and num="+QString::number(point);
    pointQuery += " ORDER BY id DESC;";
    tv->setModel(pointModel);
}

void SQLDriver::updateGroupArchive(const QDate &from, const QDate &to, QTableView *tv, int gr)
{
    QMutexLocker locker(&mutex);

    groupQuery = "SELECT tmr,cnt,di1,di2,di3,do1,do2,ip FROM gates "
            "where tmr between '";
    groupQuery += from.toString("yyyy-MM-dd 00:00:00");
    groupQuery += "' and '";
    groupQuery += to.toString("yyyy-MM-dd 23:59:59");
    groupQuery += "' and num="+QString::number(gr);
    groupQuery += " ORDER BY id DESC;";
    tv->setModel(groupModel);
}

void SQLDriver::updatePointAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point)
{
    QMutexLocker locker(&mutex);

    pointAlarmQuery = "SELECT tmr,ip,type,alarm FROM point_alarms "
            "where tmr between '";
    pointAlarmQuery += from.toString("yyyy-MM-dd 00:00:00");
    pointAlarmQuery += "' and '";
    pointAlarmQuery += to.toString("yyyy-MM-dd 23:59:59");
    pointAlarmQuery += "' and gate="+QString::number(gr);
    pointAlarmQuery += " and point="+QString::number(point);
    pointAlarmQuery += " ORDER BY id DESC;";
    tv->setModel(pointAlarmModel);
}

void SQLDriver::updateGroupAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr)
{
    QMutexLocker locker(&mutex);

    groupAlarmQuery = "SELECT tmr,ip,type,alarm FROM gate_alarms "
            "where tmr between '";
    groupAlarmQuery += from.toString("yyyy-MM-dd 00:00:00");
    groupAlarmQuery += "' and '";
    groupAlarmQuery += to.toString("yyyy-MM-dd 23:59:59");
    groupAlarmQuery += "' and gate="+QString::number(gr);
    groupAlarmQuery += " ORDER BY id DESC;";
    tv->setModel(groupAlarmModel);
}

void SQLDriver::addMessageToJournal()
{
    mutex.lock();
    if(!messages.empty()) {
        Message m = messages.front();
        QSqlQuery query;
        query.prepare("INSERT INTO journal (message, type)"
                      "VALUES (:message, :type);");
        query.bindValue(":message", m.text);
        query.bindValue(":type", m.type);
        query.exec();
        messages.pop_front();
    }
    mutex.unlock();
}

void SQLDriver::addPointAlarm(const QString &ip_addr, quint8 point_num, quint8 gate_num, const QString &message, const QString &type)
{
    QSqlQuery query;
    query.prepare("INSERT INTO point_alarms (alarm, type, point, gate, ip)"
                                          "VALUES (:alarm, :type, :point, :gate, :ip);");
    query.bindValue(":alarm", message);
    query.bindValue(":type", type);
    query.bindValue(":point", point_num);
    query.bindValue(":gate", gate_num);
    query.bindValue(":ip", ip_addr);
    query.exec();
}

void SQLDriver::addGateAlarm(const QString &ip_addr, quint8 gate_num, const QString &message, const QString &type)
{
    QSqlQuery query;
    query.prepare("INSERT INTO gate_alarms (alarm, type, gate, ip)"
                                          "VALUES (:alarm, :type, :gate, :ip);");
    query.bindValue(":alarm", message);
    query.bindValue(":type", type);
    query.bindValue(":gate", gate_num);
    query.bindValue(":ip", ip_addr);
    query.exec();
}

void SQLDriver::work()
{
    db = QSqlDatabase::addDatabase("QODBC");
    for(;;) {
        mutex.lock();
        bool finishFlagState = finishFlag;
        bool initFlagState = initFlag;
        bool insertFlagState = insertFlag;
        bool insertGroupFlagState = insertGroupFlag;
        mutex.unlock();
        if(initFlagState) {
            mutex.lock();
            initFlag = false;
            mutex.unlock();
            initDataBase();
            if(finishFlagState) break;
            QThread::msleep(1);
        }
        if(insertFlagState && db.isOpen()) {
            mutex.lock();
            insertFlag = false;
            mutex.unlock();
            insertDatatoDataBase();
        }
        if(insertGroupFlagState && db.isOpen()) {
            mutex.lock();
            insertGroupFlag = false;
            mutex.unlock();
            insertGroupDatatoDataBase();
        }
        addMessageToJournal();
        mutex.lock();
        if(!journalQuery.isEmpty()) {
            QSqlQuery query(journalQuery);
            journalModel->setQuery(query);
            journalModel->setHeaderData(0, Qt::Horizontal, tr("Время"));
            journalModel->setHeaderData(1, Qt::Horizontal, tr("Тип"));
            journalModel->setHeaderData(2, Qt::Horizontal, tr("Сообщение"));
            journalQuery="";
        }
        if(!pointQuery.isEmpty()) {
            QSqlQuery query(pointQuery);
            pointModel->setQuery(query);
            pointModel->setHeaderData(0, Qt::Horizontal, tr("Время"));
            pointModel->setHeaderData(1, Qt::Horizontal, tr("Вход 1"));
            pointModel->setHeaderData(2, Qt::Horizontal, tr("Вход 2"));
            pointModel->setHeaderData(3, Qt::Horizontal, tr("Выход 1"));
            pointModel->setHeaderData(4, Qt::Horizontal, tr("Выход 2"));
            pointModel->setHeaderData(5, Qt::Horizontal, tr("Питание"));
            pointModel->setHeaderData(6, Qt::Horizontal, tr("Аккумулятор"));
            pointModel->setHeaderData(7, Qt::Horizontal, tr("Динамики"));
            pointModel->setHeaderData(8, Qt::Horizontal, tr("IP"));
            pointQuery="";
        }
        if(!groupQuery.isEmpty()) {
            QSqlQuery query(groupQuery);
            groupModel->setQuery(query);
            groupModel->setHeaderData(0, Qt::Horizontal, tr("Время"));
            groupModel->setHeaderData(1, Qt::Horizontal, tr("Подкл. точки"));
            groupModel->setHeaderData(2, Qt::Horizontal, tr("Вход 1"));
            groupModel->setHeaderData(3, Qt::Horizontal, tr("Вход 2"));
            groupModel->setHeaderData(4, Qt::Horizontal, tr("Вход 3"));
            groupModel->setHeaderData(5, Qt::Horizontal, tr("Выход 1"));
            groupModel->setHeaderData(6, Qt::Horizontal, tr("Выход 2"));
            groupModel->setHeaderData(7, Qt::Horizontal, tr("IP"));
            groupQuery="";
        }
        if(!groupAlarmQuery.isEmpty()) {
            QSqlQuery query(groupAlarmQuery);
            groupAlarmModel->setQuery(query);
            groupAlarmModel->setHeaderData(0, Qt::Horizontal, tr("Время"));
            groupModel->setHeaderData(1, Qt::Horizontal, tr("IP"));
            groupAlarmModel->setHeaderData(2, Qt::Horizontal, tr("Тип"));
            groupAlarmModel->setHeaderData(3, Qt::Horizontal, tr("Сообщение"));
            groupAlarmQuery="";
        }
        if(!pointAlarmQuery.isEmpty()) {
            QSqlQuery query(pointAlarmQuery);
            pointAlarmModel->setQuery(query);
            pointAlarmModel->setHeaderData(0, Qt::Horizontal, tr("Время"));
            pointAlarmModel->setHeaderData(1, Qt::Horizontal, tr("IP"));
            pointAlarmModel->setHeaderData(2, Qt::Horizontal, tr("Тип"));
            pointAlarmModel->setHeaderData(3, Qt::Horizontal, tr("Сообщение"));
            pointAlarmQuery="";
        }
        mutex.unlock();
        QThread::msleep(10);
    }
}

