#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTextCodec>
#include "norwegianwoodstyle.h"
#include "dialogconfig.h"
#include <QProcess>
#include <QMessageBox>
#include "dialogdate.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include "audiotree.h"
#include <QStringList>
#include <QStyle>
#include "dialogvolumeconfig.h"
#include "dialoginputsconfig.h"
#include "pointdata.h"
#include "groupdata.h"

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

void MainWindow::setTimerInterval(int value)
{
    if(speakerTimer!=nullptr) {
        delete speakerTimer;
        speakerTimer = nullptr;
    }
    speakerTimer = new QTimer(this);
    connect(speakerTimer, SIGNAL(timeout()), this, SLOT(checkAudio()));
    if(value) {
        speakerTimer->setInterval(value*1000);
        speakerTimer->start();
    }
}

void MainWindow::updatePointsList()
{
  QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
  if (layout) {
    ui->treeWidget->clear();
    if(tree!=nullptr) delete tree;
    tree = new AudioTree(ui->treeWidget, prConfig.get());
    tree->createTree();
    layout->addStretch(1);
  }
  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->collapseAll();
  prConfig->readConfig();
  ip1 = prConfig->ip1.toInt();
  ip2 = prConfig->ip2.toInt();
  ip3 = prConfig->ip3.toInt();
  ip4 = prConfig->ip4.toInt();
  int audio_tmr = prConfig->tmr.toInt();
  int grCnt = static_cast<int>(prConfig->gates.size());
  setTimerInterval(60*audio_tmr);
  ui->comboBoxGroups->clear();
  for(int i=0;i<grCnt;i++) {
      auto grName = tree->getGroupValue(i,"name");
      if(grName) {
          ui->comboBoxGroups->addItem(std::any_cast<QString>(grName.value()));
      }
  }
}

void MainWindow::updateAlarmList(const QStringList &list)
{
    ui->listWidgetAlarm->clear();
    QStringList newList;
    if(linkState==false && buttonCmd==ButtonState::STOP) {
        newList.append("АВАРИЯ (Обрыв Ethernet связи)");
        if(alarmFlag==false) {
            alarmFlag = true;
            if(ui->checkBoxAlarm->isChecked() && (!ui->checkBoxSound->isChecked())) sound->play();
        }
    }
    newList.append(list);
    if(newList.isEmpty()) {
        ui->listWidgetAlarm->addItem("Параметры в норме");
        ui->listWidgetAlarm->item(0)->setBackground(QColor(0,255,0,150));
    }else {
        ui->listWidgetAlarm->addItems(newList);
        for(int i=0;i<newList.length();i++) {
            if(newList.at(i).toLower().contains("авария")) ui->listWidgetAlarm->item(i)->setBackground(QColor(255,0,0,150));
            else if(newList.at(i).toLower().contains("предупреждение")) ui->listWidgetAlarm->item(i)->setBackground(QColor(255,255,0,150));
        }
    }
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {

  tree = nullptr;

  prConfig = std::make_unique<ProjectConfig>("conf.json");

  QTextCodec *utfcodec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(utfcodec);
  ui->setupUi(this);

  fromDate = QDate::currentDate();
  toDate = QDate::currentDate();
  ui->lineEditFrom->setText(fromDate.toString("dd-MM-yyyy"));
  ui->lineEditTo->setText(toDate.toString("dd-MM-yyyy"));

  ui->checkBoxSound->setEnabled(false);
  ui->pushButtonMicrophone->setEnabled(false);

  qRegisterMetaType<uint8_t>("uint8_t");

  speakerTimer = nullptr;
  udpScanner = nullptr;

  recorder = new MP3Recorder();
  manager = new SQLManager();
  connect(manager,&SQLManager::error,this,&MainWindow::sqlError);
  connect(manager,&SQLManager::updateAlarmList,this,&MainWindow::updateAlarmList);
  manager->initDB();
  manager->insertMessage("Запуск приложения","сообщение");

  prConfig->readConfig();
  int gate_cnt = static_cast<int>(prConfig->gates.size());
  for(int i=0;i<gate_cnt;i++) {
      manager->setPointCnt(static_cast<quint8>(i),static_cast<quint8>(prConfig->gates.at(static_cast<std::size_t>(i)).points.size()));
  }

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
    QString ip = QString::number(static_cast<quint8>(ip1)) + ".";
    ip += QString::number(static_cast<quint8>(ip2)) + ".";
    ip += QString::number(static_cast<quint8>(ip3)) + ".";
    ip += QString::number(static_cast<quint8>(ip4));
    manager->setIP(ip);
    udpScanner = new UDPController(ip);
    udpScanner->setToID(static_cast<quint8>(linkGroup),static_cast<quint8>(linkPoint));


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
    connect(udpScanner, &UDPController::updateState,this,&MainWindow::updateState);
    connect(udpScanner, &UDPController::updateGroupState,this,&MainWindow::updateGroupState);
    connect(udpScanner, &UDPController::startRecord,this,&MainWindow::startRecord);
    connect(udpScanner, &UDPController::stopRecord,this,&MainWindow::stopRecord);

    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :lightgray;}");
    ui->pushButtonMicrophone->setStyle(new QCommonStyle);
    ui->pushButtonMicrophone->setStyleSheet("QPushButton {"
        "border: 2px solid #8f8f91;"
        "border-radius: 30px;"
        "background-color: lightgray;"
        "outline: none;"
         "padding: 10px;"
    "}"
    "QPushButton:pressed {"
        "background-color: lightgreen;"
    "}");
    ui->pushButtonMicrophone->setFlat(true);
    on_pushButtonMicrophone_released();


    ui->toolBar->addAction(QIcon(":/images/config.png"),"Настройка",[this](){QDialog *dialog = new DialogConfig(prConfig.get());auto res = dialog->exec();if(res==QDialog::Accepted) updatePointsList();delete dialog;});
    ui->toolBar->addAction(QIcon(":/images/volume.png"),"Громкость точек",[this](){
        if (buttonCmd == ButtonState::STOP) {
            DialogVolumeConfig *dialog = new DialogVolumeConfig();
            if(tree!=nullptr) {
                QStringList groups;
                int gateCnt = static_cast<int>(prConfig->gates.size());
                for(int i=0;i<gateCnt;i++) {
                    auto grName = tree->getGroupValue(i,"name");
                    if(grName) groups.append(std::any_cast<QString>(grName.value()));
                    quint8 point_cnt = static_cast<quint8>(prConfig->gates.at(static_cast<std::size_t>(i)).points.size());
                    QStringList points;
                    QStringList volumes;
                    for(int j=0;j<point_cnt;j++) {
                        auto pointName = tree->getPointValue(i,j,"name");
                        if(pointName) points.append(std::any_cast<QString>(pointName.value()));
                        auto volume = tree->getPointValue(i,j,"volume");
                        if(volume) volumes.append(std::any_cast<QString>(volume.value()));
                    }
                    dialog->addPoints(points);
                    dialog->addVolume(volumes);
                }
                dialog->addGroups(groups);
            }
            auto res = dialog->exec();
            if(res==QDialog::Accepted) {
                if(udpScanner && dialog->getCurrentVolume()>=0) {
                    if(!dialog->isAllPointsActive()) {
                        udpScanner->setVolume(dialog->getCurrentGroup()+1,dialog->getCurrentPoint()+1,dialog->getCurrentVolume());
                    }else {
                        int cnt = dialog->getCurrentPointCnt();
                        udpScanner->setVolume(dialog->getCurrentGroup()+1,cnt,dialog->getCurrentVolume(),true);
                    }
                }
            }
            delete dialog;
        }else QMessageBox::information(this,"Настройка громкости точки","Необходимо запустить опрос");
    });

    ui->toolBar->addAction(QIcon(":/images/contact.png"),"Фильтр входов точек",[this](){
        if (buttonCmd == ButtonState::STOP) {
            DialogInputsConfig *dialog = new DialogInputsConfig();
            if(tree!=nullptr) {
                int gateCnt = static_cast<int>(prConfig->gates.size());
                for(int i=0;i<gateCnt;i++) {
                    GateConf conf;
                    auto grName = tree->getGroupValue(i,"name");
                    if(grName) conf.name = std::any_cast<QString>(grName.value());
                    quint8 point_cnt = static_cast<quint8>(prConfig->gates.at(static_cast<std::size_t>(i)).points.size());
                    for(int j=0;j<point_cnt;j++) {
                        PointConf pConf;
                        auto pointName = tree->getPointValue(i,j,"name");
                        if(pointName) pConf.name = std::any_cast<QString>(pointName.value());
                        auto inp1 = tree->getPointValue(i,j,"di1");
                        auto inp2 = tree->getPointValue(i,j,"di2");
                        auto filter1 = tree->getPointValue(i,j,"di1_filter");
                        auto filter2 = tree->getPointValue(i,j,"di2_filter");
                        auto inp1Value = std::any_cast<Input>(inp1.value());
                        if(inp1Value!=Input::UNUSED) pConf.inp1En=true;else pConf.inp1En=false;
                        auto inp2Value = std::any_cast<Input>(inp2.value());
                        if(inp2Value!=Input::UNUSED) pConf.inp2En=true;else pConf.inp2En=false;
                        auto filter1Value = std::any_cast<double>(filter1.value());
                        pConf.in1Filter = static_cast<quint8>(filter1Value*2);
                        auto filter2Value = std::any_cast<double>(filter2.value());
                        pConf.in2Filter = static_cast<quint8>(filter2Value*2);
                        conf.points.push_back(pConf);
                    }
                    dialog->addGateConf(conf);
                }
                auto res = dialog->exec();
                if(res==QDialog::Accepted) {
                    quint8 grNum = dialog->getCurrentGroup();
                    quint8 pNum = dialog->getCurrentPoint();
                    quint8 filter = dialog->getFilterValue();
                    quint8 enValue = dialog->getEnableValue();
                    if(udpScanner) {
                        udpScanner->setInpConf(grNum,pNum,filter,enValue);
                    }
                }
            }
        }else QMessageBox::information(this,"Настройка входов точки","Необходимо запустить опрос");
    });

    QFont font = QFont ("Courier");
    font.setStyleHint (QFont::Monospace);
    font.setPointSize (12);
    font.setFixedPitch (true);
    ui->listWidgetAlarm->setFont(font);

    on_radioButtonAllPoints_clicked();

    alarmStartTime = QDateTime::currentSecsSinceEpoch();
    sound = new QSound(":/sounds/alarm.wav");
    sound->setLoops(QSound::Infinite);
    //timer->start();

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


    if (buttonCmd == ButtonState::START) {
        manager->insertMessage("Запуск опроса","сообщение");
        m_audioInputDevice.reset(new AudioInputDevice(format,udpScanner));
        connect(m_audioInputDevice.data(),&AudioInputDevice::newLevel,this,&MainWindow::newLevel);
        m_qaudioInput.reset(new QAudioInput(inpDeviceInfo, format));
        m_qaudioInput->setVolume(1.0);
        m_audioInputDevice->start();
        m_qaudioInput->start(m_audioInputDevice.data());


        m_audiOutputDevice.reset(new AudioOutputDevice(this));
        m_qaudioOutput.reset(new QAudioOutput(getOutDevice(ui->comboBoxOut->currentText()),format));
        //m_audiOutputDevice->start();
        //m_qaudioOutput->start(m_audiOutputDevice.data());
        connect(udpScanner,&UDPController::updateAudio,m_audiOutputDevice.data(),&AudioOutputDevice::updateAudio);
        connect(m_audiOutputDevice.data(),&AudioOutputDevice::newOutLevel,this,&MainWindow::newOutLevel);

        buttonCmd = ButtonState::STOP;
        ui->pushButtonStartStop->setText("      Стоп      ");

        m_audiOutputDevice->start();
        m_qaudioOutput->start(m_audiOutputDevice.data());
        m_qaudioOutput->resume();

        ui->comboBoxInput->setEnabled(false);
        ui->comboBoxOut->setEnabled(false);
        QString ip = QString::number(ip1)+"."+QString::number(ip2)+"."+QString::number(ip3)+"."+QString::number(ip4);
        udpScanner->setIP(ip);
        manager->setIP(ip);
        udpScanner->start();
        linkState = false;
        QTimer::singleShot(1000, this, [this](){
            if(linkState==false) {
                ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :red; }");
                manager->insertMessage("нет связи","авария");
                updateAlarmList(QStringList());
            }
        });
        sound->stop();
        alarmFlag = false;

        ui->checkBoxSound->setEnabled(true);
        ui->pushButtonMicrophone->setEnabled(true);


    }else {        
        recorder->runFlag = false;
        recorder->stopRecord();

        manager->insertMessage("Остановка опроса","сообщение");
        m_qaudioInput->suspend();
        m_audioInputDevice->stop();
        //m_qaudioOutput->suspend();
        //m_audiOutputDevice->stop();
        buttonCmd = ButtonState::START;
        ui->pushButtonStartStop->setText("      Старт      ");

        ui->comboBoxInput->setEnabled(true);
        ui->comboBoxOut->setEnabled(true);

        udpScanner->stop();


        ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :lightgray; }");
        sound->stop();
        alarmFlag = false;

        ui->checkBoxSound->setEnabled(false);
        ui->pushButtonMicrophone->setEnabled(false);
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
    //qDebug() << value;
  if(value) {
      manager->insertMessage("Связь установлена","сообщение");
    linkState = true;
    ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :green; }");
  }else {
    manager->insertMessage("Нет связи","авария");
    linkState = false;
    updateAlarmList(QStringList());
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

void MainWindow::updateState(const QByteArray state)
{
    QStringList alarmList;
    QStringList pointAlarms;
    manager->insertData(state);

    quint8 req_num = static_cast<quint8>(state.at(0));
    quint16 used_point_cnt = static_cast<quint16>((static_cast<quint16>(static_cast<quint8>(state.at(1)))<<8) | static_cast<quint8>(state.at(2)));
    int req_points_cnt = (state.length()-3)/9;
    if(used_point_cnt>req_points_cnt) return;//used_point_cnt=static_cast<quint16>(req_points_cnt);
    if(req_num==0) {
        alarmPointList.clear();
        int pointDataSize = PointData::getRawDataSize();
        for(int i=0;i<used_point_cnt;i++) {
            PointData p(state.mid(3+i*pointDataSize,pointDataSize));
            quint8 grNum = p.getGroupNum();
            quint8 pointNum = p.getPointNum();
            if(grNum && pointNum) {
                QString inp2TypeStr;
                Inp2Type inp2type = p.getInput2Type();
                if(inp2type==Inp2Type::KSL) inp2TypeStr = "КСЛ";
                else if(inp2type==Inp2Type::FENCE) inp2TypeStr = "ПЕРЕЕЗД";
                else if(inp2type==Inp2Type::JAMMING) inp2TypeStr = "ОГРАЖДЕНИЕ";
                else if(inp2type==Inp2Type::CROSSING) inp2TypeStr = "ЗАШТЫБОВКА";

                tree->setPointValue(grNum-1,pointNum-1,"di2_type",inp2TypeStr);

                quint8 vers = p.getVersion();
                if(vers<=200) tree->setPointValue(grNum-1,pointNum-1,"version",QString::number(vers)+QString(".0"));
                else tree->setPointValue(grNum-1,pointNum-1,"version","Загрузчик " + QString::number(vers-200)+QString(".0"));
                quint8 volume = p.getSoundReduction();
                if(volume==0) tree->setPointValue(grNum-1,pointNum-1,"volume",QString("максимум"));
                else if(volume>3) tree->setPointValue(grNum-1,pointNum-1,"volume",QString("некорректное значение") + QString::number(volume));
                else tree->setPointValue(grNum-1,pointNum-1,"volume",QString("1/")+QString::number(pow(2,volume)));

                auto grName = tree->getGroupValue(grNum-1,"name");
                auto pointName = tree->getPointValue(grNum-1,pointNum-1,"name");

                Input di1 = p.getInput1();
                tree->setPointValue(grNum-1,pointNum-1,"di1",di1);
                if(di1!=Input::UNUSED) {
                    if(di1==Input::BREAK) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД1(КТВ) - ОБРЫВ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }else if(di1==Input::SHORT) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД1(КТВ) - ЗАМЫКАНИЕ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }else if(di1==Input::OFF) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД1(КТВ) - ВЫКЛ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }
                }

                Input di2 = p.getInput2();
                tree->setPointValue(grNum-1,pointNum-1,"di2",di2);
                if(di2!=Input::UNUSED) {
                    if(di2==Input::BREAK) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД2(" + inp2TypeStr + ") - ОБРЫВ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }else if(di2==Input::SHORT) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД2(" + inp2TypeStr + ") - ЗАМЫКАНИЕ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }else if(di2==Input::OFF) {
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ ВХОД2(" + inp2TypeStr + ") - ВЫКЛ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }
                }


                tree->setPointValue(grNum-1,pointNum-1,"do1",p.getOutput1());
                tree->setPointValue(grNum-1,pointNum-1,"do2",p.getOutput2());
                tree->setPointValue(grNum-1,pointNum-1,"limit_switch",p.getLastPointSwitch());
                tree->setPointValue(grNum-1,pointNum-1,"di1_filter",p.getDI1Filter());
                tree->setPointValue(grNum-1,pointNum-1,"di2_filter",p.getDI2Filter());

                auto speaker = p.getSpeaker();
                tree->setPointValue(grNum-1,pointNum-1,"speaker",speaker);
                if(speaker==Speaker::PROBLEM) {
                    if(grName) {
                        QString alarmText = std::any_cast<QString>(grName.value())+" ";
                        if(pointName) {
                            alarmText += std::any_cast<QString>(pointName.value())+": ";
                            alarmText += "АВАРИЯ НЕИСПРАВНОСТЬ ДИНАМИКОВ";
                            alarmPointList.append(alarmText);
                        }
                    }
                }

                tree->setPointValue(grNum-1,pointNum-1,"power",p.getPowerVoltage());
                tree->setPointValue(grNum-1,pointNum-1,"battery",p.getAccumulatorVoltage());
            }
        }
    }
    QStringList alarms;
    if(alarmGroupList.size() || alarmPointList.size()) {
        alarms = alarmGroupList;
        alarms.append(alarmPointList);
    }
    if(alarms.isEmpty()) {
        alarmFlag = false;
        sound->stop();
    }
    else {
        if(alarmFlag==false) {
            alarmStartTime=QDateTime::currentSecsSinceEpoch();
            if(ui->checkBoxAlarm->isChecked() && (!ui->checkBoxSound->isChecked())) sound->play();
        }
        alarmFlag = true;
    }
    updateAlarmList(alarms);
}

void MainWindow::updateGroupState(const QByteArray state)
{
    quint8 rawDataSize = GroupData::getRawGroupDataSize();
    quint8 configGateSize = static_cast<quint8>(prConfig->gates.size());
    int minBufSize = prConfig->maxGateQuantity * rawDataSize;
    alarmGroupList.clear();
    if(state.length()>=minBufSize) {
        manager->insertGroupData(state);
        for(int i=0;i<configGateSize;i++) {
            GroupData grData(state.mid(i*rawDataSize,rawDataSize));
            quint8 grNum = grData.getGrNum();
            if(grNum) {
                quint8 cnt = grData.getPointsQuantity();
                tree->setGroupValue(i,"real_point_cnt",static_cast<int>(cnt));
                if(cnt<tree->pointCount(i)) {
                    auto grName = tree->getGroupValue(i,"name");
                    alarmGroupList.append(std::any_cast<QString>(grName.value()) + ":");
                    alarmGroupList.append("АВАРИЯ: Число подключенных точек - " + QString::number(cnt));
                    alarmGroupList.append("ожидается " + QString::number(tree->pointCount(i)));
                    alarmGroupList.append("");
                    for(int j=cnt;j<tree->pointCount(i);j++) {
                        tree->setPointToDefault(i,j);
                    }
                }
                tree->setGroupValue(i,"di1",grData.getInput1());
                tree->setGroupValue(i,"di2",grData.getInput2());
                tree->setGroupValue(i,"di3",grData.getInput3());
                tree->setGroupValue(i,"do1",grData.getOut1());
                tree->setGroupValue(i,"do2",grData.getOut2());
                tree->setGroupValue(i,"not_actual",grData.getNotActual());
            }else {
                if(tree->pointCount(i)) {
                    auto grName = tree->getGroupValue(i,"name");
                    alarmGroupList.append(std::any_cast<QString>(grName.value()) + ":");
                    alarmGroupList.append("АВАРИЯ: Число подключенных точек - не известно");
                    alarmGroupList.append("ожидается " + QString::number(tree->pointCount(i)));
                    alarmGroupList.append("");
                }
            }
        }
    }
}

void MainWindow::checkAudio()
{
    on_pushButtonCheckAudio_clicked();
}

void MainWindow::startRecord(uint8_t gr, uint8_t point)
{
    m_audiOutputDevice->startRecordCmd(gr,point);
    //qDebug() << gr << point;
    QString res;
    if(gr && point) {
        auto grName = tree->getGroupValue(gr-1,"name");
        auto pointName = tree->getPointValue(gr-1,point-1,"name");
        if(grName && pointName) {
            res+= std::any_cast<QString>(grName.value()) + "   (" + std::any_cast<QString>(pointName.value())+")";
            ui->lineEditInputPoint->setText(res);
        }
    }

}

void MainWindow::stopRecord()
{
    m_audiOutputDevice->stopRecordCmd();
    ui->lineEditInputPoint->setText("");
}

void MainWindow::sqlError(const QString &message)
{
    QMessageBox::critical(nullptr, tr("VOIP ДИспетчер"),message);
}

void MainWindow::radioButton_toggled(bool checked)
{
    Q_UNUSED(checked)
    /*if(checked) {
        QRadioButton *rb = dynamic_cast<QRadioButton*>(sender());
        if(rb) {
            int id = rb->property("id").toInt();
            udpScanner->setToID(static_cast<quint8>(id));
        }
    }*/
}

void MainWindow::on_pushButtonCloseTree_clicked()
{
    ui->treeWidget->collapseAll();
}

void MainWindow::on_pushButtonOpenTree_clicked()
{
    ui->treeWidget->expandAll();
}

void MainWindow::on_pushButtonCheckAudio_clicked()
{
    udpScanner->checkAudio();
}

void MainWindow::on_pushButtonJournal_clicked()
{
    int num = ui->tabWidgetArchive->currentIndex();
    if(num==0) {
        manager->updateJournal(fromDate,toDate,ui->tableViewJournal);
        ui->tableViewJournal->show();
    }else if(num==1) {
        manager->updatePointArchive(fromDate,toDate,ui->tableViewPoints,ui->spinBoxGroup->value(),ui->spinBoxPoint->value());
        ui->tableViewPoints->show();
    }else if(num==3) {
        manager->updateGroupArchive(fromDate,toDate,ui->tableViewGroups,ui->spinBoxGroup->value());
        ui->tableViewGroups->show();
    }else if(num==2) {
        manager->updatePointAlarmArchive(fromDate,toDate,ui->tableViewPointAlarm,ui->spinBoxGroup->value(),ui->spinBoxPoint->value());
        ui->tableViewPointAlarm->show();
    }else if(num==4) {
        manager->updateGroupAlarmArchive(fromDate,toDate,ui->tableViewGroupAlarm,ui->spinBoxGroup->value());
        ui->tableViewGroupAlarm->show();
    }

}

void MainWindow::on_pushButtonEndTime_clicked()
{
    DialogDate *dialog = new DialogDate(this);
    dialog->setDate(toDate);
    connect(dialog,&DialogDate::finished,dialog,&DialogDate::deleteLater);
    connect(dialog,&DialogDate::accepted,[this,dialog](){toDate = dialog->getDate();ui->lineEditTo->setText(toDate.toString("dd-MM-yyyy"));});
    dialog->show();
}

void MainWindow::on_pushButtonStartTime_clicked()
{
    DialogDate *dialog = new DialogDate(this);
    dialog->setDate(fromDate);
    connect(dialog,&DialogDate::finished,dialog,&DialogDate::deleteLater);
    connect(dialog,&DialogDate::accepted,[this,dialog](){fromDate = dialog->getDate();ui->lineEditFrom->setText(fromDate.toString("dd-MM-yyyy"));});
    dialog->show();
}

void MainWindow::on_tabWidgetArchive_currentChanged(int index)
{
    if(index==0) {
        ui->spinBoxGroup->setVisible(false);
        ui->spinBoxPoint->setVisible(false);
    }else if(index==3 || index==4){
        ui->spinBoxGroup->setVisible(true);
        ui->spinBoxPoint->setVisible(false);
    }else {
        ui->spinBoxGroup->setVisible(true);
        ui->spinBoxPoint->setVisible(true);
    }
}

void MainWindow::on_radioButtonPoint_clicked()
{
    ui->scrollArea->setEnabled(true);
    ui->comboBoxGroups->setEnabled(true);

    QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    int p_num = 0;
    if(layout) {
        QLayoutItem *item;
        int cnt = layout->count();
        for(int i=0;i<cnt;i++) {
            item = layout->itemAt(i);
            QWidget* widget =  item->widget();
            QRadioButton *rb = dynamic_cast<QRadioButton*>(widget);
            if(rb && rb->isChecked()) {p_num=i;break;}
        }
    }

    linkGroup = ui->comboBoxGroups->currentIndex()+1;
    linkPoint = p_num+1;
    udpScanner->setToID(static_cast<quint8>(linkGroup),static_cast<quint8>(linkPoint));
}

void MainWindow::on_radioButtonAllPoints_clicked()
{
    ui->scrollArea->setEnabled(false);
    ui->comboBoxGroups->setEnabled(false);

    linkGroup = 1;
    linkPoint = 128;
    udpScanner->setToID(linkGroup,linkPoint);
}

void MainWindow::on_comboBoxGroups_currentIndexChanged(int index)
{
    QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
          delete item->widget();
          delete item;
        }
        if(ui->radioButtonGroup->isChecked()) {
            linkGroup = (index+1) | (1<<7);
            linkPoint = 1;
            if(udpScanner) udpScanner->setToID(static_cast<quint8>(linkGroup),static_cast<quint8>(linkPoint));
        }
        prConfig->readConfig();
        if(index>=0) {
            int pointCnt = static_cast<int>(prConfig->gates.at(static_cast<std::size_t>(index)).points.size());
            for(int i=0;i<pointCnt;i++) {
                auto pointName = tree->getPointValue(ui->comboBoxGroups->currentIndex(),i,"name");
                if(pointName) {
                    QRadioButton *rb = new QRadioButton(std::any_cast<QString>(pointName.value()));
                    connect(rb, &QRadioButton::toggled, [=](){
                        if(ui->radioButtonPoint->isChecked()) {
                            linkGroup = ui->comboBoxGroups->currentIndex()+1;linkPoint = i+1;
                            if(udpScanner) udpScanner->setToID(static_cast<quint8>(linkGroup),static_cast<quint8>(linkPoint));
                        }

                    });
                    layout->addWidget(rb);
                    if(i==0) rb->setChecked(true);
                }
            }
            layout->addStretch();
        }
    }
}


void MainWindow::on_checkBoxAlarm_clicked(bool checked)
{
    if(checked) {
        if(alarmFlag && (!ui->checkBoxSound->isChecked())) sound->play();
    }else {
        sound->stop();
    }
}

void MainWindow::on_radioButtonGroup_clicked()
{

    ui->scrollArea->setEnabled(false);
    ui->comboBoxGroups->setEnabled(true);
    linkGroup = (ui->comboBoxGroups->currentIndex()+1)|(1<<7);
    linkPoint = 1;
    if(udpScanner) udpScanner->setToID(static_cast<quint8>(linkGroup),static_cast<quint8>(linkPoint));

    //on_comboBoxGroups_currentIndexChanged(ui->comboBoxGroups->currentIndex());
}

void MainWindow::on_pushButtonMicrophone_pressed()
{
    udpScanner->setSilentMode(false);
    ui->pushButtonMicrophone->setIcon(QIcon(":/images/mic_on.png"));
    manager->insertMessage("РЕЖИМ РАЗГОВОРА","сообщение");
    bool runFlag = false;
    if(buttonCmd == ButtonState::STOP) runFlag = true;
    recorder->runFlag = runFlag;
    recorder->stopRecordCmd = false;
    recorder->stopRecord();
    recorder->setAudioInputName(ui->comboBoxInput->currentText());
    recorder->newRecord();
}

void MainWindow::on_pushButtonMicrophone_released()
{
    bool runFlag = false;
    if(buttonCmd == ButtonState::STOP) runFlag = true;
    recorder->runFlag = runFlag;
    recorder->stopRecordCmd = true;
    recorder->stopRecord();
    udpScanner->setSilentMode(true);
    ui->pushButtonMicrophone->setIcon(QIcon(":/images/mic_off.png"));
    manager->insertMessage("РЕЖИМ ПРОСЛУШИВАНИЯ","сообщение");
}

void MainWindow::on_checkBoxSound_clicked()
{
    if(ui->checkBoxSound->isChecked()) {
        m_qaudioOutput->setVolume(0);sound->stop();
    }else {
        m_qaudioOutput->setVolume(1);
        if(alarmFlag && (!ui->checkBoxSound->isChecked()) && ui->checkBoxAlarm->isChecked()) sound->play();
    }
}

