#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audioinputdevice.h"
#include "audiooutputdevice.h"
#include <QScopedPointer>
#include <udpcontroller.h>
#include <QRadioButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum { START, STOP };

    bool buttonCmd = START;

    QScopedPointer<AudioInputDevice> m_audioInputDevice;
    QScopedPointer<QAudioInput> m_qaudioInput;

    QScopedPointer<AudioOutputDevice> m_audiOutputDevice;
    QScopedPointer<QAudioOutput> m_qaudioOutput;

    UDPController *udpScanner;

    QAudioDeviceInfo getInpDevice(const QString &name);
    QAudioDeviceInfo getOutDevice(const QString &name);

    QVector<double> x, y;
    QVector<double> x2, y2;

    std::vector<QRadioButton*> points;

    bool linkState = false;

    void updatePointsList();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonStartStop_clicked();
    void newLevel(const QVector<double> &inp);
    void newOutLevel(const QVector<double> &inp);
    void linkStatechanged(bool value);
    void fromIDChanged(unsigned char value);

void radioButton_toggled(bool checked);

void on_checkBox_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
