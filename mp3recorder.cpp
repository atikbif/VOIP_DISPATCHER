#include "mp3recorder.h"
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QTimer>
#include <QAudioEncoderSettings>
#include <QDateTime>
#include <QUrl>

MP3Recorder::MP3Recorder(QObject *parent) : QObject(parent)
{
    dispRecorder = nullptr;
}

void MP3Recorder::stopRecord()
{
    if(dispRecorder) {
        dispRecorder->stop();
        delete dispRecorder;
        QString inpName = fName;
        QString outName = fName;
        outName.replace("wav","mp3");
        while(QFile::exists(outName)) {
            outName.remove(".mp3");
            outName+="_again.mp3";
        }
        QProcess *process = new QProcess;
        QDir dir(QCoreApplication::applicationDirPath());
        process->setProgram(dir.absolutePath()+"/utils/ffmpeg.exe");
        process->setArguments(QStringList() << "-loglevel" << "fatal" << "-i" << inpName << "-codec:a" << "libmp3lame" << "-qscale:a" << "5" << outName);
        process->start();
        procs.append(process);
        wafFiles.append(inpName);

        QTimer::singleShot(10000, this, [=](){if(!procs.isEmpty()) {QProcess* pr = procs.pop();pr->kill();delete pr;}
            if(!wafFiles.isEmpty()){QString file = wafFiles.pop();if(QFile::exists(file)) QFile::remove(file);}});

        dispRecorder = nullptr;
    }
}

void MP3Recorder::newRecord()
{
    QAudioEncoderSettings audioSettings;
    if(runFlag && stopRecordCmd==false) {
        dispRecorder = new QAudioRecorder;
        audioSettings.setCodec("audio/pcm");
        audioSettings.setChannelCount(1);
        audioSettings.setBitRate(128*1024);
        audioSettings.setSampleRate(8000);
        audioSettings.setEncodingMode(QMultimedia::AverageBitRateEncoding);

        dispRecorder->setAudioInput(audioInputName);
        dispRecorder->setEncodingSettings(audioSettings);
        dispRecorder->setContainerFormat("wav");
        fName = QCoreApplication::applicationDirPath() + "/audio_records/disp";
        fName+= QDateTime::currentDateTime().toString("_dd_MM_yyyy_hh_mm_ss") + ".wav";
        while(QFile::exists(fName)) {fName.remove(".wav");fName+="_again.wav";}
        QUrl url = QUrl::fromLocalFile(fName);
        dispRecorder->setOutputLocation(url);
        dispRecorder->record();
        startTime = QDateTime::currentMSecsSinceEpoch();
        QTimer::singleShot(300000, this, [=](){
            qint64 t = QDateTime::currentMSecsSinceEpoch();
            if(t-startTime>=295000) {
                stopRecord();
                newRecord();
            }
        });
    }
}
