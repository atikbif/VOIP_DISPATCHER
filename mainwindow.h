#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audioinputdevice.h"
#include "audiooutputdevice.h"
#include <QScopedPointer>
#include <udpcontroller.h>
#include <QRadioButton>
#include <QTimer>
#include <QAudioRecorder>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum { START, STOP };

    bool buttonCmd = START;

    QTimer *speakerTimer;

    QScopedPointer<AudioInputDevice> m_audioInputDevice;
    QScopedPointer<QAudioInput> m_qaudioInput;

    QScopedPointer<AudioOutputDevice> m_audiOutputDevice;
    QScopedPointer<QAudioOutput> m_qaudioOutput;

    UDPController *udpScanner;
    QAudioRecorder *dispRecorder;
    QAudioEncoderSettings audioSettings;

    QAudioDeviceInfo getInpDevice(const QString &name);
    QAudioDeviceInfo getOutDevice(const QString &name);
    void setTimerInterval(int value);



    QVector<double> x, y;
    QVector<double> x2, y2;

    std::vector<QRadioButton*> points;
    int point_cnt;
    int audio_tmr;

    bool linkState = false;

    void updatePointsList();
    void updateAlarmList(const QStringList &list);


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonStartStop_clicked();
    void newLevel(const QVector<double> &inp);
    void newOutLevel(const QVector<double> &inp);
    void linkStatechanged(bool value);
    void fromIDChanged(unsigned char value);
    void updateState(const QByteArray &state);
    void checkAudio();
    void startRecord(uint8_t gr, uint8_t point);
    void stopRecord();

void radioButton_toggled(bool checked);

void on_checkBox_clicked();

void on_pushButtonCloseTree_clicked();

void on_pushButtonOpenTree_clicked();

void on_pushButtonCheckAudio_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
