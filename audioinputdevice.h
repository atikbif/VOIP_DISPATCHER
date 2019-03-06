#ifndef AUDIOINPUTDEVICE_H
#define AUDIOINPUTDEVICE_H

#include <QObject>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <QAudioInput>
//#include "speex/speex.h"
#include "opus.h"
#include <QUdpSocket>
#include "udpcontroller.h"

#define FRAME_SIZE 160

class AudioInputDevice : public QIODevice
{
    Q_OBJECT

  const QAudioFormat m_format; // audio stream parameters
  // speex variables
  //SpeexBits bits;
  OpusEncoder *enc;
  OpusDecoder *dec;
  //void *state;
  //void *dec_state;
  QByteArray micrData;
  qint16 input[FRAME_SIZE];
  char cbits[1024];
  quint32 m_maxAmplitude = 0;   // audio data max value
  //QUdpSocket udp;
  static quint16 id;    // request id
  bool linkState = false;
  bool prevLinkState = false;
  UDPController *scanner;

  void sendUDPData(const QByteArray &input);

public:
    AudioInputDevice(const QAudioFormat &format,UDPController *scanner);
    void start();
    void stop();

    // QIODevice interface
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

signals:
    void link_state_changed(bool linkFlag);
    void newLevel(const QVector<double> plot);
};

#endif // AUDIOINPUTDEVICE_H
