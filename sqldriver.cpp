#include "sqldriver.h"
#include <QThread>
#include "QSqlQuery"
#include <QSqlRecord>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

void SQLDriver::initDataBase()
{
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
}

void SQLDriver::insertDatatoDataBase()
{
    double acc, pow;
    QString di1,di2,do1,do2,speaker;
    QString gdi1,gdi2,gdi3,gdo1,gdo2;
    double last_acc, last_pow;
    QString last_di1, last_di2, last_do1, last_do2,last_speaker;
    QString last_gdi1, last_gdi2, last_gdi3, last_gdo1, last_gdo2;

    static qint64 last_sec = 0;
    static bool next_insert = false;
    static int offset = 0;

    if(QDateTime::currentSecsSinceEpoch()-last_sec>=10 || next_insert) {
        last_sec = QDateTime::currentSecsSinceEpoch();

        // gate

        uint16_t byte_num_state = 232;
        uint16_t byte_num_break = 232 + (1)/8;
        uint16_t byte_num_short = 232 + (2)/8;
        uint8_t bit_num_state = 0;
        uint8_t bit_num_break = (1)%8;
        uint8_t bit_num_short = (2)%8;
        gdi1 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi1 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi1 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi1 = "замыкание";

        byte_num_state = 232 + (3)/8;
        byte_num_break = 232 + (4)/8;
        byte_num_short = 232 + (5)/8;
        bit_num_state = (3)%8;
        bit_num_break = (4)%8;
        bit_num_short = (5)%8;
        gdi2 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi2 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi2 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi2 = "замыкание";

        byte_num_state = 232 + (6)/8;
        byte_num_break = 232 + (7)/8;
        byte_num_short = 232 + (8)/8;
        bit_num_state = (6)%8;
        bit_num_break = (7)%8;
        bit_num_short = (8)%8;
        gdi3 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi3 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi3 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi3 = "замыкание";

        uint16_t byte_num = 232+(9)/8;
        uint16_t bit_num = (9)%8;
        if(rawData.at(byte_num)&(1<<bit_num)) gdo1="вкл";else gdo1="выкл";

        byte_num = 232+(10)/8;
        bit_num = (10)%8;
        if(rawData.at(byte_num)&(1<<bit_num)) gdo2="вкл";else gdo2="выкл";

        QSqlQuery query;
        query.prepare("INSERT INTO gates (num, ip, cnt, di1, di2, di3, do1, do2)"
                                              "VALUES (:num, :ip, :cnt, :di1, :di2, :di3, :do1, :do2);");
        query.bindValue(":num", 1);
        query.bindValue(":ip", ip);
        query.bindValue(":cnt", (quint8)rawData.at(1));
        query.bindValue(":di1", gdi1);
        query.bindValue(":di2", gdi2);
        query.bindValue(":di3", gdi3);
        query.bindValue(":do1", gdo1);
        query.bindValue(":do2", gdo2);
        query.exec();

        if((quint8)rawData.at(1)>=point_cnt) {
            addGateAlarm(ip,1,"подключено " + QString::number((quint8)rawData.at(1)) + " точек", "сообщение");
        }else {
            addGateAlarm(ip,1,"подключено " + QString::number((quint8)rawData.at(1)) + " точек", "авария");
        }

        // points

        int start_pos = 0;
        int end_pos = (quint8)rawData.at(1);

        if(next_insert) {
            start_pos = offset;
        }
        if(end_pos-start_pos>30) {
            end_pos = start_pos+30;
            offset = end_pos;
            next_insert = true;
        }else next_insert = false;
        for(int i=start_pos;i<end_pos;i++) {
            int reg_offset = 32+i*2;
            if(reg_offset>=rawData.length()) return;
            acc = (double)((quint8)rawData.at(reg_offset))/10;
            pow = (double)((quint8)rawData.at(reg_offset+1))/10;
            byte_num_state = 232 + (16+i*10+1)/8;
            byte_num_break = 232 + (16+i*10+2)/8;
            byte_num_short = 232 + (16+i*10+3)/8;
            uint16_t max = byte_num_state;
            if(byte_num_break>max) max = byte_num_break;
            if(byte_num_short>max) max = byte_num_short;
            uint8_t bit_num_state = (16+i*10+1)%8;
            uint8_t bit_num_break = (16+i*10+2)%8;
            uint8_t bit_num_short = (16+i*10+3)%8;
            if(max>=rawData.length()) return;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) di1 = "вкл";
            else if(rawData.at(byte_num_break)&(1<<bit_num_break)) {di1 = "обрыв";}
            else if(rawData.at(byte_num_short)&(1<<bit_num_short)) {di1 = "замыкание";}
            else {di1="выкл";}
            byte_num_state = 232 + (16+i*10+4)/8;
            byte_num_break = 232 + (16+i*10+5)/8;
            byte_num_short = 232 + (16+i*10+6)/8;
            max = byte_num_state;
            if(byte_num_break>max) max = byte_num_break;
            if(byte_num_short>max) max = byte_num_short;
            bit_num_state = (16+i*10+4)%8;
            bit_num_break = (16+i*10+5)%8;
            bit_num_short = (16+i*10+6)%8;
            if(max>=rawData.length()) return;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) di2 = "вкл";
            else if(rawData.at(byte_num_break)&(1<<bit_num_break)) {di2 = "обрыв";}
            else if(rawData.at(byte_num_short)&(1<<bit_num_short)) {di2 = "замыкание";}
            else {di2="выкл";}
            byte_num_state = 232 + (16+i*10+7)/8;
            if(byte_num_state>=rawData.length()) return;
            bit_num_state = (16+i*10+7)%8;;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) do1 = "вкл";else do1="выкл";
            byte_num_state = 232 + (16+i*10+8)/8;
            if(byte_num_state>=rawData.length()) return;
            bit_num_state = (16+i*10+8)%8;;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) do2 = "вкл";else do2="выкл";
            speaker = "не исправны";
            byte_num_state = 232 + (16+i*10+0)/8;
            bit_num_state = (16+i*10+0)%8;
            uint16_t byte_num_check = 232 + (16+i*10+9)/8;
            uint8_t bit_num_check = (16+i*10+9)%8;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) {speaker = "исправны";}
            if((rawData.at(byte_num_check)&(1<<bit_num_check))==0) {speaker = "не проверялись";}
            QSqlQuery query;
            query.prepare("INSERT INTO points (num, gate, di1, di2, do1, do2, speaker, pow, bat, ip)"
                                                  "VALUES (:num, :gate, :di1, :di2, :do1, :do2, :speaker, :pow, :bat, :ip);");
            query.bindValue(":num", i+1);
            query.bindValue(":gate", 1);
            query.bindValue(":di1", di1);
            query.bindValue(":di2", di2);
            query.bindValue(":do1", do1);
            query.bindValue(":do2", do2);
            query.bindValue(":speaker", speaker);
            query.bindValue(":pow", pow);
            query.bindValue(":bat", acc);
            query.bindValue(":ip", ip);
            query.exec();

            if(di1=="вкл" && speaker=="исправны") addPointAlarm(ip,i+1,1,"параметры в норме", "сообщение");
            else {
                if(di1=="обрыв") addPointAlarm(ip,i+1,1,"Обрыв DI1", "авария");
                if(di1=="замыкание") addPointAlarm(ip,i+1,1,"Замыкание DI1", "авария");
                if(di1=="выкл") addPointAlarm(ip,i+1,1,"DI1 выкл", "авария");
                if(speaker=="не проверялись") addPointAlarm(ip,i+1,1,"Динамики не проверялись", "предупреждение");
                if(speaker=="не исправны") addPointAlarm(ip,i+1,1,"Динамики не исправны", "авария");
            }
        }
    }else {
        if(rawData.length()>lastData.length()) {
            next_insert  =true;
            offset = 0;
            return;
        }

        // gates

        uint16_t byte_num_state = 232;
        uint16_t byte_num_break = 232 + (1)/8;
        uint16_t byte_num_short = 232 + (2)/8;
        uint8_t bit_num_state = 0;
        uint8_t bit_num_break = (1)%8;
        uint8_t bit_num_short = (2)%8;
        gdi1 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi1 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi1 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi1 = "замыкание";
        last_gdi1 = "выкл";
        if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_gdi1 = "вкл";
        else if(lastData.at(byte_num_break)&(1<<bit_num_break)) last_gdi1 = "обрыв";
        else if(lastData.at(byte_num_short)&(1<<bit_num_short)) last_gdi1 = "замыкание";

        byte_num_state = 232 + (3)/8;
        byte_num_break = 232 + (4)/8;
        byte_num_short = 232 + (5)/8;
        bit_num_state = (3)%8;
        bit_num_break = (4)%8;
        bit_num_short = (5)%8;
        gdi2 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi2 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi2 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi2 = "замыкание";
        last_gdi2 = "выкл";
        if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_gdi2 = "вкл";
        else if(lastData.at(byte_num_break)&(1<<bit_num_break)) last_gdi2 = "обрыв";
        else if(lastData.at(byte_num_short)&(1<<bit_num_short)) last_gdi2 = "замыкание";

        byte_num_state = 232 + (6)/8;
        byte_num_break = 232 + (7)/8;
        byte_num_short = 232 + (8)/8;
        bit_num_state = (6)%8;
        bit_num_break = (7)%8;
        bit_num_short = (8)%8;
        gdi3 = "выкл";
        if(rawData.at(byte_num_state)&(1<<bit_num_state)) gdi3 = "вкл";
        else if(rawData.at(byte_num_break)&(1<<bit_num_break)) gdi3 = "обрыв";
        else if(rawData.at(byte_num_short)&(1<<bit_num_short)) gdi3 = "замыкание";
        last_gdi3 = "выкл";
        if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_gdi3 = "вкл";
        else if(lastData.at(byte_num_break)&(1<<bit_num_break)) last_gdi3 = "обрыв";
        else if(lastData.at(byte_num_short)&(1<<bit_num_short)) last_gdi3 = "замыкание";

        uint16_t byte_num = 232+(9)/8;
        uint16_t bit_num = (9)%8;
        if(rawData.at(byte_num)&(1<<bit_num)) gdo1="вкл";else gdo1="выкл";
        if(lastData.at(byte_num)&(1<<bit_num)) last_gdo1="вкл";else last_gdo1="выкл";

        byte_num = 232+(10)/8;
        bit_num = (10)%8;
        if(rawData.at(byte_num)&(1<<bit_num)) gdo2="вкл";else gdo2="выкл";
        if(lastData.at(byte_num)&(1<<bit_num)) last_gdo2="вкл";else last_gdo2="выкл";

        if((gdi1!=last_gdi1)||(gdi2!=last_gdi2)||(gdi3!=last_gdi3)||(gdo1!=last_gdo1)||(gdo2!=last_gdo2)||((quint8)rawData.at(1)!=(quint8)lastData.at(1))) {
            QSqlQuery query;
            query.prepare("INSERT INTO gates (num, ip, cnt, di1, di2, di3, do1, do2)"
                                                  "VALUES (:num, :ip, :cnt, :di1, :di2, :di3, :do1, :do2);");
            query.bindValue(":num", 1);
            query.bindValue(":ip", ip);
            query.bindValue(":cnt", (quint8)rawData.at(1));
            query.bindValue(":di1", gdi1);
            query.bindValue(":di2", gdi2);
            query.bindValue(":di3", gdi3);
            query.bindValue(":do1", gdo1);
            query.bindValue(":do2", gdo2);
            query.exec();
        }

        if((quint8)rawData.at(1) != (quint8)lastData.at(1)) {
            if((quint8)rawData.at(1)>=point_cnt) {
                addGateAlarm(ip,1,"подключено " + QString::number((quint8)rawData.at(1)) + " точек", "сообщение");
            }else {
                addGateAlarm(ip,1,"подключено " + QString::number((quint8)rawData.at(1)) + " точек", "авария");
            }
        }

        // points

        for(int i=0;i<(quint8)rawData.at(1);i++) {
            int reg_offset = 32+i*2;
            if(reg_offset>=rawData.length()) return;
            last_acc = (double)((quint8)lastData.at(reg_offset))/10;
            last_pow = (double)((quint8)lastData.at(reg_offset+1))/10;
            acc = (double)((quint8)rawData.at(reg_offset))/10;
            pow = (double)((quint8)rawData.at(reg_offset+1))/10;

            uint16_t byte_num_state = 232 + (16+i*10+1)/8;
            uint16_t byte_num_break = 232 + (16+i*10+2)/8;
            uint16_t byte_num_short = 232 + (16+i*10+3)/8;
            uint16_t max = byte_num_state;
            if(byte_num_break>max) max = byte_num_break;
            if(byte_num_short>max) max = byte_num_short;
            uint8_t bit_num_state = (16+i*10+1)%8;
            uint8_t bit_num_break = (16+i*10+2)%8;
            uint8_t bit_num_short = (16+i*10+3)%8;
            if(max>=rawData.length()) return;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) di1 = "вкл";
            else if(rawData.at(byte_num_break)&(1<<bit_num_break)) {di1 = "обрыв";}
            else if(rawData.at(byte_num_short)&(1<<bit_num_short)) {di1 = "замыкание";}
            else {di1="выкл";}
            if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_di1 = "вкл";
            else if(lastData.at(byte_num_break)&(1<<bit_num_break)) {last_di1 = "обрыв";}
            else if(lastData.at(byte_num_short)&(1<<bit_num_short)) {last_di1 = "замыкание";}
            else {last_di1="выкл";}
            byte_num_state = 232 + (16+i*10+4)/8;
            byte_num_break = 232 + (16+i*10+5)/8;
            byte_num_short = 232 + (16+i*10+6)/8;
            max = byte_num_state;
            if(byte_num_break>max) max = byte_num_break;
            if(byte_num_short>max) max = byte_num_short;
            bit_num_state = (16+i*10+4)%8;
            bit_num_break = (16+i*10+5)%8;
            bit_num_short = (16+i*10+6)%8;
            if(max>=rawData.length()) return;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) di2 = "вкл";
            else if(rawData.at(byte_num_break)&(1<<bit_num_break)) {di2 = "обрыв";}
            else if(rawData.at(byte_num_short)&(1<<bit_num_short)) {di2 = "замыкание";}
            else {di2="выкл";}
            if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_di2 = "вкл";
            else if(lastData.at(byte_num_break)&(1<<bit_num_break)) {last_di2 = "обрыв";}
            else if(lastData.at(byte_num_short)&(1<<bit_num_short)) {last_di2 = "замыкание";}
            else {last_di2="выкл";}
            byte_num_state = 232 + (16+i*10+7)/8;
            if(byte_num_state>=rawData.length()) return;
            bit_num_state = (16+i*10+7)%8;;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) do1 = "вкл";else do1="выкл";
            if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_do1 = "вкл";else last_do1="выкл";
            byte_num_state = 232 + (16+i*10+8)/8;
            if(byte_num_state>=rawData.length()) return;
            bit_num_state = (16+i*10+8)%8;;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) do2 = "вкл";else do2="выкл";
            if(lastData.at(byte_num_state)&(1<<bit_num_state)) last_do2 = "вкл";else last_do2="выкл";
            speaker = "не исправны";last_speaker="не исправны";
            byte_num_state = 232 + (16+i*10+0)/8;
            bit_num_state = (16+i*10+0)%8;
            uint16_t byte_num_check = 232 + (16+i*10+9)/8;
            uint8_t bit_num_check = (16+i*10+9)%8;
            if(rawData.at(byte_num_state)&(1<<bit_num_state)) {speaker = "исправны";}
            if((rawData.at(byte_num_check)&(1<<bit_num_check))==0) {speaker = "не проверялись";}
            if(lastData.at(byte_num_state)&(1<<bit_num_state)) {last_speaker = "исправны";}
            if((lastData.at(byte_num_check)&(1<<bit_num_check))==0) {last_speaker = "не проверялись";}

            if((last_acc!=acc)||(last_pow!=pow)||(di1!=last_di1)||(di2!=last_di2)||(do1!=last_do1)||(do2!=last_do2)||(speaker!=last_speaker)) {
                QSqlQuery query;
                query.prepare("INSERT INTO points (num, gate, di1, di2, do1, do2, speaker, pow, bat, ip)"
                                                      "VALUES (:num, :gate, :di1, :di2, :do1, :do2, :speaker, :pow, :bat, :ip);");
                query.bindValue(":num", i+1);
                query.bindValue(":gate", 1);
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
                    if(di1=="обрыв") addPointAlarm(ip,i+1,1,"Обрыв DI1", "авария");
                    if(di1=="замыкание") addPointAlarm(ip,i+1,1,"Замыкание DI1", "авария");
                    if(di1=="выкл") addPointAlarm(ip,i+1,1,"DI1 выкл", "авария");
                    if(di1=="вкл") addPointAlarm(ip,i+1,1,"DI1 вкл", "сообщение");
                }
                if(last_speaker!=speaker) {
                    if(speaker=="не проверялись") addPointAlarm(ip,i+1,1,"Динамики не проверялись", "предупреждение");
                    if(speaker=="не исправны") addPointAlarm(ip,i+1,1,"Динамики не исправны", "авария");
                    if(speaker=="исправны") addPointAlarm(ip,i+1,1,"Динамики исправны", "сообщение");
                }
            }
        }
    }
    lastData.clear();
    lastData.append(rawData);
}

SQLDriver::SQLDriver(QObject *parent) : QObject(parent)
{

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

void SQLDriver::insertMessage(const QString &text, const QString &type)
{
    QMutexLocker locker(&mutex);
    Message message;
    message.text = text;
    message.type = type;
    messages.push_back(message);
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
    bool finishFlagState = false;
    bool initFlagState = false;
    bool insertFlagState = false;
    db = QSqlDatabase::addDatabase("QODBC");
    for(;;) {
        mutex.lock();
        finishFlagState = finishFlag;
        initFlagState = initFlag;
        insertFlagState = insertFlag;
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
        addMessageToJournal();
    }
}

