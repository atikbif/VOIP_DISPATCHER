#include "audiooutputdevice.h"
#include <QDebug>
#include <QtEndian>
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QTimer>

AudioOutputDevice::AudioOutputDevice(QObject *parent):QIODevice (parent)
{
  audioStream.append(6400,'\0');
}

void AudioOutputDevice::start()
{
  open(QIODevice::ReadOnly);
}

void AudioOutputDevice::stop()
{
    close();
}

void AudioOutputDevice::startRecordCmd(quint8 gr, quint8 p)
{
    mutex.lock();
    startRecord = true;
    gr_num = gr;
    p_num = p;
    mutex.unlock();
}

void AudioOutputDevice::stopRecordCmd()
{
    mutex.lock();
    stopRecord = true;
    mutex.unlock();
}

qint64 AudioOutputDevice::readData(char *data, qint64 maxlen)
{
  static QStack<QProcess*> procs;
  static QStack<QString> pcmFiles;
  static qint64 startTime;
  static bool restart =false;
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
    if(recordOn==false && startRecord) {
        startRecord = false;
        QString fName = QCoreApplication::applicationDirPath() + "/audio_records/gr"+QString::number(gr_num);
        fName+="_point"+QString::number(p_num);
        fName+= QDateTime::currentDateTime().toString("_dd_MM_yyyy_hh_mm_ss") + ".pcm";
        while(QFile::exists(fName)) {fName.remove(".pcm");fName+="_again.pcm";}
        recFiles.append(fName);
        recordOn = true;
        startTime = QDateTime::currentMSecsSinceEpoch();
        QTimer::singleShot(300000, this, [=](){
            qint64 t = QDateTime::currentMSecsSinceEpoch();
            if(t-startTime>=295000) {
                restart = true;
            }
        });
    }
    if(recordOn) {
        recordStream.append(QByteArray::fromRawData(data,maxlen));

        if(stopRecord || restart) {
            stopRecord = false;
            recordOn = false;

            QString fName = recFiles.pop();
            QFile file(fName);
            file.open(QIODevice::WriteOnly);
            file.write(recordStream);
            file.close();
            QString inpName = fName;
            QString outName = fName;
            outName.replace("pcm","mp3");
            while(QFile::exists(outName)) {
                outName.remove(".mp3");
                outName+="_again.mp3";
            }
            QProcess *process = new QProcess;
            QDir dir(QCoreApplication::applicationDirPath());
            process->setProgram(dir.absolutePath()+"/utils/ffmpeg.exe");
            process->setArguments(QStringList() << "-loglevel" << "fatal" << "-f" << "s16le" << "-ar" << "8k" << "-ac" << "1" << "-i" << inpName <<
                                  "-filter:a" << "volume=2.0" << "-codec:a" << "libmp3lame" << "-qscale:a" << "5" << outName);
            process->start();
            //qDebug() << process->arguments();
            procs.append(process);
            pcmFiles.append(fName);

            QTimer::singleShot(10000, this, [=](){if(!procs.isEmpty()) {QProcess* pr = procs.pop();pr->kill();delete pr;}
                if(!pcmFiles.isEmpty()){QString file = pcmFiles.pop();if(QFile::exists(file)) QFile::remove(file);}});

            recordStream.clear();
        }
        if(restart) {
            restart = false;
            startRecord = true;
        }
    }
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
  while (readPos>updPos && abs(updPos - readPos) < 640 + data.count()) {
    //qDebug() << "!" << updPos << readPos << data.count();
    //updPos+=data.count()*2;
    //if(updPos>=audioStream.count()) updPos-= audioStream.count();
    if(readPos>=data.count()) readPos-=data.count();else break;
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
