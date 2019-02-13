#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audioinputdevice.h"
#include <QScopedPointer>
#include <udpcontroller.h>

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

    UDPController *udpScanner;

    QAudioDeviceInfo getInpDevice(const QString &name);
    QAudioDeviceInfo getOutDevice(const QString &name);

    QVector<double> x, y;

    bool linkState = false;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonStartStop_clicked();
    void newLevel(const QVector<double> &inp);
    void linkStatechanged(bool value);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
