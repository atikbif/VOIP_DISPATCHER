#ifndef UDPCONTROLLER_H
#define UDPCONTROLLER_H

#include <QObject>
#include <QThread>
#include "udpworker.h"

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

signals:
    void init();
    void linkStateChanged(bool value);
public slots:
};

#endif // UDPCONTROLLER_H
