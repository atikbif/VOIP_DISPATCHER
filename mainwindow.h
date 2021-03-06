#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audioinputdevice.h"
#include "audiooutputdevice.h"
#include <QScopedPointer>
#include <udpcontroller.h>
#include <QRadioButton>
#include "sqlmanager.h"
#include "audiotree.h"
#include <QSound>
#include <QTimer>
#include "mp3recorder.h"
#include "projectconfig.h"
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT



    enum class ButtonState { START, STOP };

    ButtonState buttonCmd = ButtonState::START;

    QTimer *speakerTimer;
    AudioTree *tree;

    QSound *sound;

    QScopedPointer<AudioInputDevice> m_audioInputDevice;
    QScopedPointer<QAudioInput> m_qaudioInput;

    QScopedPointer<AudioOutputDevice> m_audiOutputDevice;
    QScopedPointer<QAudioOutput> m_qaudioOutput;

    UDPController *udpScanner;
    QAudioEncoderSettings audioSettings;

    QDate fromDate;
    QDate toDate;

    std::unique_ptr<ProjectConfig> prConfig;

    qint64 alarmStartTime;
    bool alarmFlag = false;

    QAudioDeviceInfo getInpDevice(const QString &name);
    QAudioDeviceInfo getOutDevice(const QString &name);
    void setTimerInterval(int value);   // периодическая автоматическая проверка динамиков



    QVector<double> x, y;
    QVector<double> x2, y2;

    std::vector<QRadioButton*> points;
    int point_cnt;
    int audio_tmr;

    bool linkState = false;

    void updatePointsList();


    int ip1,ip2,ip3,ip4;
    QStringList alarmGroupList;
    QStringList alarmPointList;

    int linkGroup=0;
    int linkPoint=0;

    SQLManager *manager;
    MP3Recorder *recorder;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateAlarmList(const QStringList &list);

private slots:
    void on_pushButtonStartStop_clicked();
    void newLevel(const QVector<double> &inp);
    void newOutLevel(const QVector<double> &inp);
    void linkStatechanged(bool value);
    void fromIDChanged(unsigned char value);
    void updateState(const QByteArray state);
    void updateGroupState(const QByteArray state);
    void checkAudio();
    void startRecord(uint8_t gr, uint8_t point);
    void stopRecord();
    void sqlError(const QString &message);

void radioButton_toggled(bool checked);
void on_pushButtonCloseTree_clicked();
void on_pushButtonOpenTree_clicked();
void on_pushButtonCheckAudio_clicked();
void on_pushButtonJournal_clicked();
void on_pushButtonEndTime_clicked();
void on_pushButtonStartTime_clicked();
void on_tabWidgetArchive_currentChanged(int index);
void on_radioButtonPoint_clicked();
void on_radioButtonAllPoints_clicked();
void on_comboBoxGroups_currentIndexChanged(int index);
void on_checkBoxAlarm_clicked(bool checked);
void on_radioButtonGroup_clicked();
void on_pushButtonMicrophone_pressed();
void on_pushButtonMicrophone_released();
void on_checkBoxSound_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
