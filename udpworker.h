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
    bool volumeCmd = false;
    int volumeGroup = 0;
    int volumePoint = 0;
    int volumeValue = 0;
    QByteArray packet;

    //SpeexBits bits;
    //void *state;
    //void *dec_state;
    OpusEncoder *enc;
    OpusDecoder *dec;

    mutable QMutex mutex;
    static quint16 id;
    static const int wait_time_ms = 30;
    //quint8 toID = 0xFF;
    quint8 fromID = 0x00;

    quint8 fromGroup;
    quint8 fromPoint;

    quint8 grId = 0;
    quint8 pointId = 0;

    bool startFlag = false;
    qint64 startTime;

    QByteArray createRequestWriteAudio(const QByteArray &input, bool silentMode = false);
    QByteArray createRequestSetVolume(quint8 group, quint8 point, quint8 value);
    QByteArray createRequestCheckLink();
    QByteArray createRequestReadState();
    QByteArray createRequestCheckAudio();
    bool silent = false;
    bool checkAudioFlag = false;

    void checkLink(QUdpSocket &udp);
    void readState(QUdpSocket &udp);
    void checkAudioCmd(QUdpSocket &udp);
    QString ip;

public:
    explicit UDPWorker(const QString &ip, QObject *parent = nullptr);
    void start();
    void stop();
    void finish();
    void writeAudioPacket(const QByteArray &input);
    bool getLinkState() const;
    void setToID(unsigned char group, unsigned char point) {/*toID = id;*/grId=group;pointId=point;}
    void setSilentMode(bool value) {silent=value;}
    void setIP(const QString &value) {ip=value;}
    void checkAudio();
    void setVolume(int group,int point, int value);

signals:
  void linkStateChanged(bool value);
  void updateAudio(QByteArray data);
  void updateState(const QByteArray data);
  void updateGroupState(const QByteArray data);
  void fromIDSignal(unsigned char value);
  void startRecord(uint8_t gr, uint8_t point);
  void stopRecord();
public slots:
    void scan();

};

#endif // UDPWORKER_H
