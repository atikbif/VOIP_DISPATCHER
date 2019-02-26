#ifndef AUDIOOUTPUTDEVICE_H
#define AUDIOOUTPUTDEVICE_H

#include <QObject>
#include <QIODevice>
#include <QMutex>
#include <QByteArray>
#include <QAudioOutput>

class AudioOutputDevice : public QIODevice
{
  Q_OBJECT

  QByteArray audioStream;
  QByteArray inputStream;
  int updPos = 0;
  int readPos = 0;
  int curOutBufNum = 1;
  QByteArray allData;
  QMutex mutex;
public:
    AudioOutputDevice(QObject *parent=nullptr);
    void start();
    void stop();
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
public:
    qint64 bytesAvailable() const;
public slots:
    void updateAudio(QByteArray data);
signals:
    void newOutLevel(const QVector<double> plot);
};

#endif // AUDIOOUTPUTDEVICE_H
