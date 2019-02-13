#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include<QTextCodec>


QAudioDeviceInfo MainWindow::getInpDevice(const QString &name)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
      if(name == deviceInfo.deviceName()) return deviceInfo;
    }
    return QAudioDeviceInfo::defaultInputDevice();
}

QAudioDeviceInfo MainWindow::getOutDevice(const QString &name)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
      if(name == deviceInfo.deviceName()) return deviceInfo;
    }
    return QAudioDeviceInfo::defaultOutputDevice();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QTextCodec *utfcodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utfcodec);
    ui->setupUi(this);




    QStringList inpDevices;
    const QAudioDeviceInfo &defaultInpDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    inpDevices.append(defaultInpDeviceInfo.deviceName());
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        QString devName = deviceInfo.deviceName();
        if(!inpDevices.contains(devName))  inpDevices.append(devName);
    }
    for(QString name:inpDevices) {ui->comboBoxInput->addItem(name);}


    QStringList outDevices;
    const QAudioDeviceInfo &defaultOutDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    outDevices.append(defaultOutDeviceInfo.deviceName());
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        QString devName = deviceInfo.deviceName();
        if(!outDevices.contains(devName)) outDevices.append(devName);
    }
    for(QString name:outDevices) {ui->comboBoxOut->addItem(name);}
    udpScanner = new UDPController();


    for(int i = 0; i < 1000; i++)
    {
       x.append(i);
       y.append(0);
    }

    ui->widget->xAxis->setTickLabels(false);
    ui->widget->yAxis->setTickLabels(false);
    ui->widget->addGraph();
    ui->widget->graph(0)->setData(x,y);
    ui->widget->xAxis->setRange(0, 1000);
    ui->widget->yAxis->setRange(-1, 1);
    ui->widget->replot();

    connect(udpScanner, &UDPController::linkStateChanged,this,&MainWindow::linkStatechanged);

    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :lightgray; }");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonStartStop_clicked()
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    auto inpDeviceInfo = getInpDevice(ui->comboBoxInput->currentText());
    if (!inpDeviceInfo.isFormatSupported(format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        format = inpDeviceInfo.nearestFormat(format);
        qWarning() << format.channelCount() << " " << format.codec() << " " << format.sampleRate() << " " << format.sampleSize() << " " << format.sampleType() << " " << format.byteOrder();
    }


    if (buttonCmd == START) {
        m_audioInputDevice.reset(new AudioInputDevice(format,udpScanner));
        connect(m_audioInputDevice.data(),&AudioInputDevice::newLevel,this,&MainWindow::newLevel);
        m_qaudioInput.reset(new QAudioInput(inpDeviceInfo, format));
        m_audioInputDevice->start();
        m_qaudioInput->start(m_audioInputDevice.data());

        buttonCmd = STOP;
        ui->pushButtonStartStop->setText("Стоп");

        ui->comboBoxInput->setEnabled(false);
        ui->comboBoxOut->setEnabled(false);

        udpScanner->start();
        linkState = false;

        QTimer::singleShot(1000, this, [this](){if(linkState==false) ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :red; }");});


    }else {

        m_qaudioInput->suspend();
        m_audioInputDevice->stop();
        buttonCmd = START;
        ui->pushButtonStartStop->setText("Старт");

        ui->comboBoxInput->setEnabled(true);
        ui->comboBoxOut->setEnabled(true);

        udpScanner->stop();


        ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :lightgray; }");
    }
}

void MainWindow::newLevel(const QVector<double> &inp)
{
    QVector<double> x2, y2;
    for(const auto v: inp) {
        y.pop_front();
        y.append(v);
    }
    ui->widget->graph(0)->setData(x, y);
    ui->widget->replot();
}

void MainWindow::linkStatechanged(bool value)
{
  if(value) {
    linkState = true;
    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :green; }");
  }else {
    linkState = false;
    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :red; }");
  }
}
