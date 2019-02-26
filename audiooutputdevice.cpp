#include "audiooutputdevice.h"
#include <QDebug>
#include <QtEndian>

AudioOutputDevice::AudioOutputDevice(QObject *parent):QIODevice (parent)
{
  audioStream.append(3200,'\0');
}

void AudioOutputDevice::start()
{
  open(QIODevice::ReadOnly);
}

void AudioOutputDevice::stop()
{
  close();
}

qint64 AudioOutputDevice::readData(char *data, qint64 maxlen)
{
  if (maxlen >= 640) maxlen = 640;
  mutex.lock();
  for(int i=0;i<maxlen;i++) {
        data[i] = audioStream.at(readPos);
        audioStream[readPos] = 0;
        readPos++;
        if(readPos>=audioStream.count()) readPos = 0;
    }
    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    QVector<double> plot;
    for (int i = 0; i < maxlen/2; ++i) {
        qint16 value = qFromLittleEndian<qint16>(ptr);
        ptr += 2;
        plot.append((double)value/32767);
    }
    emit newOutLevel(plot);
    mutex.unlock();
    return maxlen;
}

qint64 AudioOutputDevice::writeData(const char *data, qint64 len)
{
  Q_UNUSED(data)
  Q_UNUSED(len)
  return 0;
}

qint64 AudioOutputDevice::bytesAvailable() const
{
  return 0;
}

void AudioOutputDevice::updateAudio(QByteArray data)
{
  mutex.lock();
  while (abs(updPos - readPos) < 640 + data.count()) {
  updPos+=data.count();
      if(updPos>=audioStream.count()) updPos-= audioStream.count();
  }
  int remain = audioStream.count() - updPos;
  if(remain>=data.count()) {
      audioStream.replace(updPos,data.count(),data);
      updPos+=data.count();
      //if(updPos>=audioStream.count()) updPos-= audioStream.count();
  }else {

      if(remain) audioStream.replace(updPos,remain,data);
      int secondPartLength = data.count() - remain;
      audioStream.replace(0,secondPartLength,data.mid(remain,secondPartLength));
      updPos = secondPartLength;
  }
  mutex.unlock();
}
