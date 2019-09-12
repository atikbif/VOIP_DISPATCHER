#ifndef SQLDRIVER_H
#define SQLDRIVER_H

#include <QObject>
#include <QMutex>
#include "QtSql/QSqlDatabase"
#include <deque>

class SQLDriver : public QObject
{
    struct Message {
        QString text;
        QString type;
    };

    Q_OBJECT
    QString ip;
    quint8 point_cnt=0;
    mutable QMutex mutex;
    bool finishFlag = false;
    bool initFlag = false;
    bool insertFlag = false;
    std::deque<Message> messages;
    QByteArray rawData;
    QByteArray lastData;
    QSqlDatabase db;

    void initDataBase();
    void insertDatatoDataBase();
    void addMessageToJournal();
    void addPointAlarm(const QString &ip_addr, quint8 point_num, quint8 gate_num, const QString &message, const QString &type);
    void addGateAlarm(const QString &ip_addr, quint8 gate_num, const QString &message, const QString &type);

public:
    explicit SQLDriver(QObject *parent = nullptr);
    void finish();
    void initDB();
    void insertData(const QByteArray &data);
    void insertMessage(const QString &text, const QString &type);
    void setIP(const QString &value) {ip=value;}
    void setPointCnt(quint8 value) {point_cnt=value;}

signals:
    void error(const QString &message);
    void updateAlarmList(const QStringList &list);
public slots:
    void work();
};

#endif // SQLDRIVER_H
