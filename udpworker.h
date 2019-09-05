#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QMutex>
#include <QUdpSocket>
#include <QByteArray>
//#include "speex/speex.h"
#include "opus.h"
#include <QDateTime>

class UDPWorker : public QObject
{
    Q_OBJECT
    bool workFlag = false;
    bool finishFlag = false;
    bool newAudioPacketFlag = false;
    bool linkState = false;
    QByteArray packet;

    //SpeexBits bits;
    //void *state;
    //void *dec_state;
    OpusEncoder *enc;
    OpusDecoder *dec;

    mutable QMutex mutex;
    static quint16 id;
    static const int wait_time_ms = 30;
    quint8 toID = 0xFF;
    quint8 fromID = 0x00;

    bool startFlag = false;
    qint64 startTime;

    QByteArray createRequestWriteAudio(const QByteArray &input, bool silentMode = false);
    QByteArray createRequestCheckLink();
    QByteArray createRequestReadState();
    QByteArray createRequestCheckAudio();
    bool silent = false;
    bool checkAudioFlag = false;

    void checkLink(QUdpSocket &udp);
    void readState(QUdpSocket &udp);
    void checkAudioCmd(QUdpSocket &udp);

public:
    explicit UDPWorker(QObject *parent = nullptr);
    void start();
    void stop();
    void finish();
    void writeAudioPacket(const QByteArray &input);
    bool getLinkState() const;
    void setToID(unsigned char id) {toID = id;}
    void setSilentMode(bool value) {silent=value;}
    void checkAudio();

signals:
  void linkStateChanged(bool value);
  void updateAudio(QByteArray data);
  void updateState(const QByteArray &data);
  void fromIDSignal(unsigned char value);
  void startRecord(uint8_t gr, uint8_t point);
  void stopRecord();
public slots:
    void scan();

};

#endif // UDPWORKER_H
