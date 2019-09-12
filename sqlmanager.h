#ifndef SQLMANAGER_H
#define SQLMANAGER_H

#include <QObject>
#include "sqldriver.h"
#include <QThread>

class SQLManager : public QObject
{
    Q_OBJECT
    SQLDriver *driver;
    QThread sqlThread;
public:
    explicit SQLManager(QObject *parent = nullptr);
    ~SQLManager();
    void initDB();
    void insertData(const QByteArray &data);
    void insertMessage(const QString &text, const QString &type);
    void setIP(const QString &value);
    void setPointCnt(quint8 value);

signals:
    void init();
    void error(const QString &message);
    void updateAlarmList(const QStringList &list);
public slots:
};

#endif // SQLMANAGER_H
