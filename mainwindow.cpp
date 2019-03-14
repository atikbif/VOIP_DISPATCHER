#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include<QTextCodec>
#include "norwegianwoodstyle.h"
#include "dialogconfig.h"

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

void MainWindow::updatePointsList()
{
  QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
  if (layout) {
    int cur_id = 0;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
      QRadioButton *rb = dynamic_cast<QRadioButton*>(item->widget());
      if(rb) {
        if(rb->isChecked()) cur_id = rb->property("id").toInt();
      }
      delete item->widget();
      delete item;
    }
    QRadioButton *rb = new QRadioButton("ШИРОКОВЕЩАТЕЛЬНЫЙ");
    rb->setChecked(true);
    connect(rb, &QRadioButton::toggled, this, &MainWindow::radioButton_toggled);
    rb->setProperty("id", 0xFF);
    layout->addWidget(rb);

    QFile confFile("conf.json");
      if (confFile.open(QIODevice::ReadOnly)) {
        QByteArray saveData = confFile.readAll();
        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject loadOb = loadDoc.object();
        if (loadOb.contains("points") && loadOb["points"].isArray()) {
          QJsonArray jsPoints = loadOb["points"].toArray();
        for (int i = 0; i < jsPoints.size(); ++i) {
          QJsonObject pointOb = jsPoints[i].toObject();
          bool checkPoint = true;
          if(!pointOb.contains("address") || !pointOb["address"].isDouble()) checkPoint = false;
          if(!pointOb.contains("name") || !pointOb["name"].isString()) checkPoint = false;
          if(!pointOb.contains("on") || !pointOb["on"].isBool()) checkPoint = false;
          if(checkPoint) {
            int num = pointOb["address"].toInt()-1;
            if(num>=0) {
              if(pointOb["on"].toBool()) {
                QRadioButton *rb = new QRadioButton(pointOb["name"].toString());
                points.push_back(rb);
                connect(rb,&QRadioButton::toggled,this,&MainWindow::radioButton_toggled);
                rb->setProperty("id", pointOb["address"].toInt());
                rb->setToolTip("Адрес: " + QString::number(pointOb["address"].toInt()));
                rb->setStyleSheet("QRadioButton {color:black;font-weight: normal;}");
                layout->addWidget(rb);
                if(cur_id && pointOb["address"].toInt()==cur_id) rb->setChecked(true);
              }
            }
          }
        }
      }
    }
    layout->addStretch(1);
  }
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  QTextCodec *utfcodec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(utfcodec);
  ui->setupUi(this);

    updatePointsList();
    QApplication::setStyle(new NorwegianWoodStyle);


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
       x2.append(i);
       y2.append(0);
    }

    ui->widget->xAxis->setTickLabels(false);
    ui->widget->yAxis->setTickLabels(false);
    ui->widget->addGraph();
    ui->widget->graph(0)->setData(x,y);
    ui->widget->xAxis->setRange(0, 1000);
    ui->widget->yAxis->setRange(-1, 1);
    ui->widget->replot();

    ui->widget_out->xAxis->setTickLabels(false);
    ui->widget_out->yAxis->setTickLabels(false);
    ui->widget_out->addGraph();
    ui->widget_out->graph(0)->setData(x2, y2);
    ui->widget_out->xAxis->setRange(0, 1000);
    ui->widget_out->yAxis->setRange(-1, 1);
    ui->widget_out->replot();

    connect(udpScanner, &UDPController::linkStateChanged,this,&MainWindow::linkStatechanged);
    connect(udpScanner, &UDPController::fromIDSignal,this,&MainWindow::fromIDChanged);

    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :lightgray; }");

    ui->toolBar->addAction(QIcon(":/images/config.png"),"Настройка",[this](){QDialog *dialog = new DialogConfig();auto res = dialog->exec();if(res==QDialog::Accepted) updatePointsList();delete dialog;});
    //showMaximized();
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
        m_qaudioInput->setVolume(0.5);
        m_audioInputDevice->start();
        m_qaudioInput->start(m_audioInputDevice.data());


        m_audiOutputDevice.reset(new AudioOutputDevice(this));
        m_qaudioOutput.reset(new QAudioOutput(getOutDevice(ui->comboBoxOut->currentText()),format));
        //m_audiOutputDevice->start();
        //m_qaudioOutput->start(m_audiOutputDevice.data());
        connect(udpScanner,&UDPController::updateAudio,m_audiOutputDevice.data(),&AudioOutputDevice::updateAudio);
        connect(m_audiOutputDevice.data(),&AudioOutputDevice::newOutLevel,this,&MainWindow::newOutLevel);

        buttonCmd = STOP;
        ui->pushButtonStartStop->setText("Стоп");

        m_audiOutputDevice->start();
        m_qaudioOutput->start(m_audiOutputDevice.data());
        m_qaudioOutput->resume();

        ui->comboBoxInput->setEnabled(false);
        ui->comboBoxOut->setEnabled(false);

        udpScanner->start();
        linkState = false;

        QTimer::singleShot(1000, this, [this](){if(linkState==false) ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :red; }");});


    }else {

        m_qaudioInput->suspend();
        m_audioInputDevice->stop();
        //m_qaudioOutput->suspend();
        //m_audiOutputDevice->stop();
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
    for(const auto v: inp) {
        y.pop_front();
        y.append(v);
    }
    ui->widget->graph(0)->setData(x, y);
    ui->widget->replot();
}

void MainWindow::newOutLevel(const QVector<double> &inp)
{
  for (const auto v : inp) {
    y2.pop_front();
    y2.append(v);
  }
  ui->widget_out->graph(0)->setData(x2, y2);
  ui->widget_out->replot();
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

void MainWindow::fromIDChanged(unsigned char value)
{
    if(value==0) {
        for(QRadioButton* rb:points) rb->setStyleSheet("QRadioButton {color:black;}");
    }else {
        for(QRadioButton *rb:points) {
            int id = rb->property("id").toInt();
            if(id==value) rb->setStyleSheet("QRadioButton {color:darkgreen;font-weight: bold;}");
            else rb->setStyleSheet("QRadioButton {color:black;font-weight: normal;}");
        }
    }
}

void MainWindow::radioButton_toggled(bool checked)
{
    if(checked) {
        QRadioButton *rb = dynamic_cast<QRadioButton*>(sender());
        if(rb) {
            int id = rb->property("id").toInt();
            qDebug() << id;
            udpScanner->setToID(id);
        }
    }
}

void MainWindow::on_checkBox_clicked()
{
    udpScanner->setSilentMode(ui->checkBox->isChecked());
}
