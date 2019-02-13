#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QMutex>
#include <QUdpSocket>

class UDPWorker : public QObject
{
    Q_OBJECT
    bool workFlag = false;
    bool finishFlag = false;
    bool newAudioPacketFlag = false;
    bool linkState = false;
    QByteArray packet;

    mutable QMutex mutex;
    static quint16 id;
    static const int wait_time_ms = 10;


    QByteArray createRequestWriteAudio(const QByteArray &input);
    QByteArray createRequestCheckLink();

    void checkLink(QUdpSocket &udp);

public:
    explicit UDPWorker(QObject *parent = nullptr);
    void start();
    void stop();
    void finish();
    void writeAudioPacket(const QByteArray &input);
    bool getLinkState() const;

signals:
  void linkStateChanged(bool value);

public slots:
    void scan();
};

#endif // UDPWORKER_H
