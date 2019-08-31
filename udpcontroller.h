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
    explicit UDPController(QObject *parent = nullptr);
    ~UDPController();
    void start();
    void stop();
    void writeAudioPacket(const QByteArray &input);
    void setToID(unsigned char id);
    void setSilentMode(bool value) {worker->setSilentMode(value);}
    void checkAudio();

signals:
    void init();
    void linkStateChanged(bool value);
    void updateAudio(QByteArray data);
    void fromIDSignal(unsigned char value);
    void updateState(const QByteArray &data);
public slots:
};

#endif // UDPCONTROLLER_H
