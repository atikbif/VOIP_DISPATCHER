#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QMutex>
#include <QUdpSocket>
#include <QByteArray>
#include "speex/speex.h"

class UDPWorker : public QObject
{
    Q_OBJECT
    bool workFlag = false;
    bool finishFlag = false;
    bool newAudioPacketFlag = false;
    bool linkState = false;
    QByteArray packet;

    SpeexBits bits;
    void *state;
    void *dec_state;

    mutable QMutex mutex;
    static quint16 id;
    static const int wait_time_ms = 10;
    quint8 toID = 0xFF;
    quint8 fromID = 0x00;

    QByteArray createRequestWriteAudio(const QByteArray &input, bool silentMode = false);
    QByteArray createRequestCheckLink();
    bool silent = false;

    void checkLink(QUdpSocket &udp);

public:
    explicit UDPWorker(QObject *parent = nullptr);
    void start();
    void stop();
    void finish();
    void writeAudioPacket(const QByteArray &input);
    bool getLinkState() const;
    void setToID(unsigned char id) {toID = id;}
    void setSilentMode(bool value) {silent=value;}

signals:
  void linkStateChanged(bool value);
  void updateAudio(QByteArray data);
  void fromIDSignal(unsigned char value);
public slots:
    void scan();

};

#endif // UDPWORKER_H
