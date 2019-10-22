#ifndef SQLDRIVER_H
#define SQLDRIVER_H

#include <QObject>
#include <QMutex>
#include "QtSql/QSqlDatabase"
#include <deque>
#include <QSqlQueryModel>
#include <QTableView>
#include "coloredsqlquerymodel.h"
#include <array>

class SQLDriver : public QObject
{
    struct Message {
        QString text;
        QString type;
    };

    Q_OBJECT
    QString ip;
    std::vector<quint8> point_cnt;
    mutable QMutex mutex;
    bool finishFlag = false;
    bool initFlag = false;
    bool insertFlag = false;
    bool insertGroupFlag = false;
    QString journalQuery;
    QString pointQuery;
    QString groupQuery;
    QString pointAlarmQuery;
    QString groupAlarmQuery;
    std::deque<Message> messages;
    QByteArray rawData;
    QByteArray rawGroupData;
    QByteArray lastData;
    QByteArray lastGroupData;
    QSqlDatabase db;
    std::array<bool,256> groupCorrectDataFlag;
    /*QSqlQueryModel *journalModel;
    QSqlQueryModel *pointModel;
    QSqlQueryModel *groupModel;
    QSqlQueryModel *groupAlarmModel;
    QSqlQueryModel *pointAlarmModel;*/
    ColoredSqlQueryModel *journalModel=nullptr;
    ColoredSqlQueryModel *pointModel=nullptr;
    ColoredSqlQueryModel *groupModel=nullptr;
    ColoredSqlQueryModel *groupAlarmModel=nullptr;
    ColoredSqlQueryModel *pointAlarmModel=nullptr;

    void initDataBase();
    void insertDatatoDataBase();
    void insertGroupDatatoDataBase();
    void addMessageToJournal();
    void addPointAlarm(const QString &ip_addr, quint8 point_num, quint8 gate_num, const QString &message, const QString &type);
    void addGateAlarm(const QString &ip_addr, quint8 gate_num, const QString &message, const QString &type);


public:
    explicit SQLDriver(QObject *parent = nullptr);
    void finish();
    void initDB();
    void insertData(const QByteArray &data);
    void insertGroupData(const QByteArray &data);
    void insertMessage(const QString &text, const QString &type);
    void setIP(const QString &value) {ip=value;}
    void setPointCnt(quint8 grNum, quint8 value) {if(grNum<point_cnt.size()) point_cnt[grNum]=value;}
    void updateJournal(const QDate &from, const QDate &to, QTableView *tv);
    void updatePointArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point);
    void updateGroupArchive(const QDate &from, const QDate &to, QTableView *tv, int gr);
    void updatePointAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point);
    void updateGroupAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr);

signals:
    void error(const QString &message);
    void updateAlarmList(const QStringList &list);
public slots:
    void work();
};

#endif // SQLDRIVER_H
