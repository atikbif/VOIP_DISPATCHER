#ifndef MP3RECORDER_H
#define MP3RECORDER_H

#include <QObject>
#include <QStack>
#include <QProcess>
#include <QString>
#include <QAudioRecorder>


class MP3Recorder : public QObject
{
    Q_OBJECT
public:
    explicit MP3Recorder(QObject *parent = nullptr);
private:
    QString fName;
    QStack<QProcess*> procs;
    QStack<QString> wafFiles;
    qint64 startTime;
    QAudioRecorder *dispRecorder;
    QString audioInputName;
public:
    bool stopRecordCmd;
    bool runFlag;

    void setAudioInputName(const QString &name) {audioInputName=name;}
    void stopRecord();
    void newRecord();
signals:

};

#endif // MP3RECORDER_H
