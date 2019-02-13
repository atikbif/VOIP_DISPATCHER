#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include "speex/speex.h"

#define FRAME_SIZE 160

class AudioInput : public QIODevice
{
  const QAudioFormat m_format;
  SpeexBits bits;
  void *state;
  void *dec_state;
  QByteArray micrData;
  qint16 input[FRAME_SIZE];
  char cbits[1024];
  quint32 m_maxAmplitude = 0;
public:
    AudioInput(const QAudioFormat &format);
    void start();
    void stop();

    // QIODevice interface
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
};

#endif // AUDIOINPUT_H
