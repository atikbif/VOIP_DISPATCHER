#include "sqlmanager.h"

SQLManager::SQLManager(QObject *parent) : QObject(parent)
{
    driver = new SQLDriver();
    driver->moveToThread(&sqlThread);
    connect(&sqlThread, &QThread::finished, driver, &QObject::deleteLater);
    connect(this, &SQLManager::init, driver, &SQLDriver::work);
    connect(driver,&SQLDriver::error,this,&SQLManager::error);
    connect(driver,&SQLDriver::updateAlarmList,this,&SQLManager::updateAlarmList);
    sqlThread.start();
    emit init();
}

SQLManager::~SQLManager()
{
    driver->finish();
    sqlThread.quit();
    sqlThread.wait();
}

void SQLManager::initDB()
{
    driver->initDB();
}

void SQLManager::insertData(const QByteArray &data)
{
    driver->insertData(data);
}

void SQLManager::insertGroupData(const QByteArray &data)
{
    driver->insertGroupData(data);
}

void SQLManager::insertMessage(const QString &text, const QString &type)
{
    driver->insertMessage(text,type);
}

void SQLManager::setIP(const QString &value)
{
    driver->setIP(value);
}

void SQLManager::setPointCnt(quint8 grNum, quint8 value)
{
    driver->setPointCnt(grNum,value);
}

void SQLManager::updateJournal(const QDate &from, const QDate &to, QTableView *tv)
{
    driver->updateJournal(from,to,tv);
}

void SQLManager::updatePointArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point)
{
    driver->updatePointArchive(from,to,tv,gr,point);
}

void SQLManager::updateGroupArchive(const QDate &from, const QDate &to, QTableView *tv, int gr)
{
    driver->updateGroupArchive(from,to,tv,gr);
}

void SQLManager::updatePointAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr, int point)
{
    driver->updatePointAlarmArchive(from,to,tv,gr,point);
}

void SQLManager::updateGroupAlarmArchive(const QDate &from, const QDate &to, QTableView *tv, int gr)
{
    driver->updateGroupAlarmArchive(from,to,tv,gr);
}
