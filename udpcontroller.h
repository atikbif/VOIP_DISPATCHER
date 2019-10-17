#ifndef UDPCONTROLLER_H
#define UDPCONTROLLER_H

#include <QObject>
#include <QThread>
#include "udpworker.h"
#include <QByteArray>

class UDPController : public QObject
{
    Q_OBJECT
    QThread udpThread;
    UDPWorker *worker;
public:
    explicit UDPController(const QString &ip, QObject *parent = nullptr);
    ~UDPController();
    void start();
    void stop();
    void writeAudioPacket(const QByteArray &input);
    void setToID(unsigned char group, unsigned char point);
    void setSilentMode(bool value) {worker->setSilentMode(value);}
    void checkAudio();
    void setIP(const QString &ip) {worker->setIP(ip);}
    void setVolume(int group,int point, int value);

signals:
    void init();
    void linkStateChanged(bool value);
    void updateAudio(QByteArray data);
    void fromIDSignal(unsigned char value);
    void updateGroupState(const QByteArray &data);
    void updateState(const QByteArray &data);
    void startRecord(uint8_t gr, uint8_t point);
    void stopRecord();
public slots:
};

#endif // UDPCONTROLLER_H
