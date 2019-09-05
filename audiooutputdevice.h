#ifndef AUDIOOUTPUTDEVICE_H
#define AUDIOOUTPUTDEVICE_H

#include <QObject>
#include <QIODevice>
#include <QMutex>
#include <QByteArray>
#include <QAudioOutput>
#include <QStack>

class AudioOutputDevice : public QIODevice
{
  Q_OBJECT

  QByteArray audioStream;
  QByteArray inputStream;
  QByteArray recordStream;
  int updPos = 0;
  int readPos = 0;
  int curOutBufNum = 1;
  quint8 gr_num = 0;
  quint8 p_num = 0;
  bool startRecord = false;
  bool stopRecord = false;
  bool recordOn = false;
  QStack<QString> recFiles;
  QByteArray allData;
  QMutex mutex;
public:
    AudioOutputDevice(QObject *parent=nullptr);
    void start();
    void stop();
    void startRecordCmd(quint8 gr, quint8 p);
    void stopRecordCmd();
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
