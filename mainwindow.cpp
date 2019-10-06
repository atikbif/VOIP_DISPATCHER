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

    ui->treeWidget->clear();
    tree = new AudioTree(ui->treeWidget);
    tree->createTree();
    layout->addStretch(1);
  }
  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->collapseAll();
  QStringList conf = readConf();
  ip1 = conf.at(0).toInt();
  ip2 = conf.at(1).toInt();
  ip3 = conf.at(2).toInt();
  ip4 = conf.at(3).toInt();
  int audio_tmr = conf.at(4).toInt();
  setTimerInterval(60*audio_tmr);
}

QStringList MainWindow::readConf()
{
    QString ip1,ip2,ip3,ip4,tmr;
    QStringList res;
    QStringList p_cnt;
    QString gate_cnt = "0";
    QFile confFile("conf.json");
    if(confFile.open(QIODevice::ReadOnly)) {
        QByteArray saveData = confFile.readAll();
        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject loadOb = loadDoc.object();
        if(loadOb.contains("ip1")) {
          ip1 = loadOb["ip1"].toString();
        }
        if(loadOb.contains("ip2")) {
          ip2 = loadOb["ip2"].toString();
        }
        if(loadOb.contains("ip3")) {
          ip3 = loadOb["ip3"].toString();
        }
        if(loadOb.contains("ip4")) {
          ip4 = loadOb["ip4"].toString();
        }
        if(loadOb.contains("audio tmr")) {
          tmr = loadOb["audio tmr"].toString();
        }
        if(loadOb.contains("gate cnt")) {
          gate_cnt = loadOb["gate cnt"].toString();
          int v = gate_cnt.toInt();
          if(v>0 && v<=32) {
              if(loadOb.contains("gates") && loadOb["gates"].isArray()) {
                QJsonArray jsGates = loadOb["gates"].toArray();
                int gate_length = jsGates.size();
                if(v>gate_length) v=gate_length;
                gate_cnt = QString::number(v);
                for (int i=0; i<v; ++i) {
                    QJsonObject gateOb = jsGates[i].toObject();
                    if(gateOb.contains("cnt")) {
                        int cnt = gateOb["cnt"].toInt();
                        p_cnt.append(QString::number(cnt));
                    }
                }
              }
          }
        }
    }
    res << ip1 << ip2 << ip3 << ip4 << tmr;
    if(gate_cnt.toInt()==p_cnt.length()) {
        res << gate_cnt;
        res << p_cnt;
    }else res << "0";
    return res;
}

void MainWindow::updateAlarmList(const QStringList &list)
{
    ui->listWidgetAlarm->clear();
    QStringList newList;
    if(linkState==false && buttonCmd==STOP) newList.append("АВАРИЯ (Обрыв Ethernet связи)");
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
  QTextCodec *utfcodec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(utfcodec);
  ui->setupUi(this);

  fromDate = QDate::currentDate();
  toDate = QDate::currentDate();
  ui->lineEditFrom->setText(fromDate.toString("dd-MM-yyyy"));
  ui->lineEditTo->setText(toDate.toString("dd-MM-yyyy"));

  qRegisterMetaType<uint8_t>("uint8_t");

  speakerTimer = nullptr;
  dispRecorder = nullptr;
  udpScanner = nullptr;

  manager = new SQLManager();
  connect(manager,&SQLManager::error,this,&MainWindow::sqlError);
  connect(manager,&SQLManager::updateAlarmList,this,&MainWindow::updateAlarmList);
  manager->initDB();
  manager->insertMessage("Запуск приложения","сообщение");

  QStringList conf = readConf();
  int gate_cnt = conf.at(5).toInt();
  if(conf.length()>=6+gate_cnt) {
      for(int i=0;i<gate_cnt;i++) {
          manager->setPointCnt(static_cast<quint8>(i),static_cast<quint8>(conf.at(6+i).toInt()));
      }
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

    ui->toolBar->addAction(QIcon(":/images/config.png"),"Настройка",[this](){QDialog *dialog = new DialogConfig();auto res = dialog->exec();if(res==QDialog::Accepted) updatePointsList();delete dialog;});

    ui->checkBox->setStyleSheet("QCheckBox::indicator {width: 64px; height: 64px; } QCheckBox::indicator:unchecked { image: url(:/speak.png); } QCheckBox::indicator:checked { image: url(:/listen.png); }");
    ui->checkBox->click();

    QFont font = QFont ("Courier");
    font.setStyleHint (QFont::Monospace);
    font.setPointSize (12);
    font.setFixedPitch (true);
    ui->listWidgetAlarm->setFont(font);
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


    if (buttonCmd == START) {
        manager->insertMessage("Запуск опроса","сообщение");
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
        QString ip = QString::number(ip1)+"."+QString::number(ip2)+"."+QString::number(ip3)+"."+QString::number(ip4);
        udpScanner->setIP(ip);
        manager->setIP(ip);
        udpScanner->start();
        linkState = false;
        QTimer::singleShot(1000, this, [this](){if(linkState==false) {ui->pushButtonStartStop->setStyleSheet("QPushButton{ background-color :red; }");manager->insertMessage("нет связи","авария");}});


    }else {
        manager->insertMessage("Остановка опроса","сообщение");
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

void MainWindow::updateState(const QByteArray &state)
{
    QStringList alarmList;
    QStringList pointAlarms;
    QList<QTreeWidgetItem *> items = ui->treeWidget->findItems(
                QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
    bool gate_alarm = false;
    /*QTreeWidgetItem *gateItem = nullptr;
    for(QTreeWidgetItem *item:items) {
        if(item->whatsThis(1)=="pcnt") {
            int cnt = static_cast<quint8>(static_cast<quint8>(state.at(0)))<<8 | static_cast<quint8>(state.at(1));
            item->setText(1,QString::number(cnt));
            item->setBackground(0, QColor(255,255,255,0));
            if(cnt<point_cnt) {
                gate_alarm = true;
                gateItem = item->parent();
                item->setBackground(0, QColor(255,0,0,150));
                item->setText(1,QString::number(cnt) + "  (ожидается "+QString::number(point_cnt)+")");
                alarmList.append(gateItem->text(0)+":");
                alarmList.append("АВАРИЯ: Число подключенных точек - " + QString::number(cnt));
                alarmList.append("ожидается " + QString::number(point_cnt));
                alarmList.append("");
            }
        }else if(item->whatsThis(1)=="gate_di1") {
            QString res = "выкл";
            uint16_t byte_num_state = 232;
            uint16_t byte_num_break = 232 + (1)/8;
            uint16_t byte_num_short = 232 + (2)/8;
            uint8_t bit_num_state = 0;
            uint8_t bit_num_break = (1)%8;
            uint8_t bit_num_short = (2)%8;
            if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
            else if(state.at(byte_num_break)&(1<<bit_num_break)) res = "обрыв";
            else if(state.at(byte_num_short)&(1<<bit_num_short)) res = "замыкание";
            item->setText(1,res);
        }else if(item->whatsThis(1)=="gate_di2") {
            QString res = "выкл";
            uint16_t byte_num_state = 232 + (3)/8;
            uint16_t byte_num_break = 232 + (4)/8;
            uint16_t byte_num_short = 232 + (5)/8;
            uint8_t bit_num_state = (3)%8;
            uint8_t bit_num_break = (4)%8;
            uint8_t bit_num_short = (5)%8;
            if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
            else if(state.at(byte_num_break)&(1<<bit_num_break)) res = "обрыв";
            else if(state.at(byte_num_short)&(1<<bit_num_short)) res = "замыкание";
            item->setText(1,res);
        }else if(item->whatsThis(1)=="gate_di3") {
            QString res = "выкл";
            uint16_t byte_num_state = 232 + (6)/8;
            uint16_t byte_num_break = 232 + (7)/8;
            uint16_t byte_num_short = 232 + (8)/8;
            uint8_t bit_num_state = (6)%8;
            uint8_t bit_num_break = (7)%8;
            uint8_t bit_num_short = (8)%8;
            if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
            else if(state.at(byte_num_break)&(1<<bit_num_break)) res = "обрыв";
            else if(state.at(byte_num_short)&(1<<bit_num_short)) res = "замыкание";
            item->setText(1,res);
        }else if(item->whatsThis(1)=="gate_do1") {
            uint16_t byte_num = 232+(9)/8;
            uint16_t bit_num = (9)%8;
            if(state.at(byte_num)&(1<<bit_num)) item->setText(1,"вкл");
            else item->setText(1,"выкл");
        }else if(item->whatsThis(1)=="gate_do2") {
            uint16_t byte_num = 232+(10)/8;
            uint16_t bit_num = (10)%8;
            if(state.at(byte_num)&(1<<bit_num)) item->setText(1,"вкл");
            else item->setText(1,"выкл");
        }
    }*/
    manager->insertData(state);
    /*for(int i=0;i<100;i++) {
        pointAlarms.clear();
        bool point_alarm = false;
        QTreeWidgetItem *pointItem = nullptr;
        QString link_audio = "p_"+QString::number(i+1)+"_audio";
        QString link_pow = "p_"+QString::number(i+1)+"_power";
        QString link_acc = "p_"+QString::number(i+1)+"_acc";
        QString link_di1 = "p_"+QString::number(i+1)+"_di1";
        QString link_di2 = "p_"+QString::number(i+1)+"_di2";
        QString link_do1 = "p_"+QString::number(i+1)+"_do1";
        QString link_do2 = "p_"+QString::number(i+1)+"_do2";
        for(QTreeWidgetItem *item:items) {
            if(item->whatsThis(1)==link_pow) {
                QString pow = QString::number(static_cast<double>(static_cast<quint8>(state.at(32+i*2+1)))/10);

                item->setText(1,pow + " В");
            }else if(item->whatsThis(1)==link_acc) {
                item->setText(1,QString::number(static_cast<quint8>(static_cast<quint8>(state.at(32+i*2)))/10)+" В");
            }else if(item->whatsThis(1)==link_di1) {
                bool alarm = false;
                QString res = "выкл";
                uint16_t byte_num_state = static_cast<uint16_t>(232 + (16+i*10+1)/8);
                uint16_t byte_num_break = static_cast<uint16_t>(232 + (16+i*10+2)/8);
                uint16_t byte_num_short = static_cast<uint16_t>(232 + (16+i*10+3)/8);
                uint8_t bit_num_state = (16+i*10+1)%8;
                uint8_t bit_num_break = (16+i*10+2)%8;
                uint8_t bit_num_short = (16+i*10+3)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                else if(state.at(byte_num_break)&(1<<bit_num_break)) {res = "обрыв";alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ОБРЫВ");}
                else if(state.at(byte_num_short)&(1<<bit_num_short)) {res = "замыкание";alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ЗАМЫКАНИЕ");}
                else {alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ВЫКЛ");}
                pointItem = item->parent();
                item->setBackground(0, QColor(255,255,255,0));
                if(alarm) {
                    point_alarm = true;
                    item->setBackground(0, QColor(255,0,0,150));
                }
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_di2) {
                QString res = "выкл";
                uint16_t byte_num_state = static_cast<uint16_t>(232 + (16+i*10+4)/8);
                uint16_t byte_num_break = static_cast<uint16_t>(232 + (16+i*10+5)/8);
                uint16_t byte_num_short = static_cast<uint16_t>(232 + (16+i*10+6)/8);
                uint8_t bit_num_state = (16+i*10+4)%8;
                uint8_t bit_num_break = (16+i*10+5)%8;
                uint8_t bit_num_short = (16+i*10+6)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                else if(state.at(byte_num_break)&(1<<bit_num_break)) {res = "обрыв";}
                else if(state.at(byte_num_short)&(1<<bit_num_short)) {res = "замыкание";}
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_do1) {
                QString res = "выкл";
                uint16_t byte_num_state = static_cast<uint16_t>(232 + (16+i*10+7)/8);
                uint8_t bit_num_state = (16+i*10+7)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_do2) {
                QString res = "выкл";
                uint16_t byte_num_state = static_cast<uint16_t>(232 + (16+i*10+8)/8);
                uint8_t bit_num_state = (16+i*10+8)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_audio) {
                bool alarm = true;
                QString res = "не исправны";
                uint16_t byte_num_state = static_cast<uint16_t>(232 + (16+i*10+0)/8);
                uint8_t bit_num_state = (16+i*10+0)%8;
                uint16_t byte_num_check = static_cast<uint16_t>(232 + (16+i*10+9)/8);
                uint8_t bit_num_check = (16+i*10+9)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) {res = "исправны";alarm=false;}
                if((state.at(byte_num_check)&(1<<bit_num_check))==0) {res = "не проверялись";alarm=false;}
                item->setText(1,res);
                item->setBackground(0, QColor(255,255,255,0));
                if(alarm) {
                    pointAlarms.append("АВАРИЯ НЕИСПРАВНОСТЬ ДИНАМИКОВ");
                    point_alarm = true;
                    item->setBackground(0, QColor(255,0,0,150));
                }
            }
        }
        if(pointItem) {
            pointItem->setBackground(0, QColor(255,255,255,0));
            gateItem = pointItem->parent();
            if(point_alarm) {
                alarmList.append(pointItem->text(0)+":");
                alarmList.append(pointAlarms);
                alarmList.append("");
                gate_alarm = true;
                pointItem->setBackground(0, QColor(255,0,0,150));
            }
        }

    }*/
    /*if(gateItem) {
        gateItem->setBackground(0, QColor(255,255,255,0));
        if(gate_alarm) {
            gateItem->setBackground(0, QColor(255,0,0,150));
        }
    }*/



    //tree->setPointValue(0,0,"di1",Input::ON);

    quint8 req_num = static_cast<quint8>(state.at(0));
    quint16 used_point_cnt = static_cast<quint16>((static_cast<quint16>(static_cast<quint8>(state.at(1)))<<8) | static_cast<quint8>(state.at(2)));
    int req_points_cnt = (state.length()-3)/7;
    if(used_point_cnt>req_points_cnt) used_point_cnt=static_cast<quint16>(req_points_cnt);
    if(req_num==0) {
        alarmPointList.clear();
        for(int i=0;i<used_point_cnt;i++) {
            quint8 grNum = static_cast<quint8>(state.at(3+i*7));
            quint8 pointNum = static_cast<quint8>(state.at(4+i*7));
            if(grNum && pointNum) {
                quint8 battery = static_cast<quint8>(state.at(5+i*7));
                quint8 power = static_cast<quint8>(state.at(6+i*7));
                tree->setPointValue(grNum-1,pointNum-1,"battery",(static_cast<double>(battery))/10);
                tree->setPointValue(grNum-1,pointNum-1,"power",(static_cast<double>(power))/10);
                quint16 bits = static_cast<quint8>(state.at(8+i*7));
                bits = static_cast<quint16>((static_cast<quint16>(bits)<<8) | static_cast<quint8>(state.at(9+i*7)));
                bool di1_state = bits & (1<<0);
                bool di1_break = bits & (1<<1);
                bool di1_short = bits & (1<<2);
                bool di2_state = bits & (1<<3);
                bool di2_break = bits & (1<<4);
                bool di2_short = bits & (1<<5);
                bool do1_state = bits & (1<<6);
                bool do2_state = bits & (1<<7);
                bool speaker_state = bits & (1<<8);
                bool speaker_check = bits & (1<<9);
                if(di1_break) {
                    tree->setPointValue(grNum-1,pointNum-1,"di1",Input::BREAK);
                    auto grName = tree->getGroupValue(grNum-1,"name");
                    if(grName) {
                        QString alarmText = std::any_cast<QString>(grName.value())+" ";
                        auto pointName = tree->getPointValue(grNum-1,pointNum-1,"name");
                        if(pointName) {
                            alarmText += std::any_cast<QString>(pointName.value())+": ";
                            alarmText += "АВАРИЯ ВХОД1(КТВ) - ОБРЫВ";
                            alarmPointList.append(alarmText);
                        }
                    }
                }
                else if(di1_short) {
                    tree->setPointValue(grNum-1,pointNum-1,"di1",Input::SHORT);
                    auto grName = tree->getGroupValue(grNum-1,"name");
                    if(grName) {
                        QString alarmText = std::any_cast<QString>(grName.value())+" ";
                        auto pointName = tree->getPointValue(grNum-1,pointNum-1,"name");
                        if(pointName) {
                            alarmText += std::any_cast<QString>(pointName.value())+": ";
                            alarmText += "АВАРИЯ ВХОД1(КТВ) - ЗАМЫКАНИЕ";
                            alarmPointList.append(alarmText);
                        }
                    }
                }
                else if(di1_state) {tree->setPointValue(grNum-1,pointNum-1,"di1",Input::ON);}
                else {
                    tree->setPointValue(grNum-1,pointNum-1,"di1",Input::OFF);
                    auto grName = tree->getGroupValue(grNum-1,"name");
                    if(grName) {
                        QString alarmText = std::any_cast<QString>(grName.value())+" ";
                        auto pointName = tree->getPointValue(grNum-1,pointNum-1,"name");
                        if(pointName) {
                            alarmText += std::any_cast<QString>(pointName.value())+": ";
                            alarmText += "АВАРИЯ ВХОД1(КТВ) - ВЫКЛ";
                            alarmPointList.append(alarmText);
                        }
                    }
                }
                if(di2_break) {tree->setPointValue(grNum-1,pointNum-1,"di2",Input::BREAK);}
                else if(di2_short) {tree->setPointValue(grNum-1,pointNum-1,"di2",Input::SHORT);}
                else if(di2_state) {tree->setPointValue(grNum-1,pointNum-1,"di2",Input::ON);}
                else {tree->setPointValue(grNum-1,pointNum-1,"di2",Input::OFF);}
                tree->setPointValue(grNum-1,pointNum-1,"do1",do1_state);
                tree->setPointValue(grNum-1,pointNum-1,"do2",do2_state);
                if(speaker_check==false) {tree->setPointValue(grNum-1,pointNum-1,"speaker",Speaker::NOT_CHECKED);}
                else {
                    if(speaker_state) {tree->setPointValue(grNum-1,pointNum-1,"speaker",Speaker::CORRECT);}
                    else {
                        tree->setPointValue(grNum-1,pointNum-1,"speaker",Speaker::PROBLEM);
                        auto grName = tree->getGroupValue(grNum-1,"name");
                        if(grName) {
                            QString alarmText = std::any_cast<QString>(grName.value())+" ";
                            auto pointName = tree->getPointValue(grNum-1,pointNum-1,"name");
                            if(pointName) {
                                alarmText += std::any_cast<QString>(pointName.value())+": ";
                                alarmText += "АВАРИЯ НЕИСПРАВНОСТЬ ДИНАМИКОВ";
                                alarmPointList.append(alarmText);
                            }
                        }
                    }
                }
            }
        }
    }
    if(alarmGroupList.size() || alarmPointList.size()) {
        QStringList alarms = alarmGroupList;
        alarms.append(alarmPointList);
        updateAlarmList(alarms);
    }
}

void MainWindow::updateGroupState(const QByteArray &state)
{
    alarmGroupList.clear();
    if(state.length()>=32*5) {
        manager->insertGroupData(state);
        for(int i=0;i<32;i++) {
            quint8 num = static_cast<quint8>(state.at(i*5+0));
            if(num) {
               // qDebug()<<num;
                quint8 cnt = static_cast<quint8>(state.at(i*5+1));
                tree->setGroupValue(i,"real_point_cnt",static_cast<int>(cnt));
                if(cnt<tree->pointCount(num-1)) {
                    auto grName = tree->getGroupValue(num-1,"name");
                    alarmGroupList.append(std::any_cast<QString>(grName.value()) + ":");
                    alarmGroupList.append("АВАРИЯ: Число подключенных точек - " + QString::number(cnt));
                    alarmGroupList.append("ожидается " + QString::number(tree->pointCount(num-1)));
                    alarmGroupList.append("");
                }
                quint16 bits = static_cast<quint8>(state.at(3+i*5));
                bits = static_cast<quint16>((static_cast<quint8>(bits)<<8) | static_cast<quint8>(state.at(4+i*5)));
                bool di1_state = bits & (1<<0);
                bool di1_break = bits & (1<<1);
                bool di1_short = bits & (1<<2);
                bool di2_state = bits & (1<<3);
                bool di2_break = bits & (1<<4);
                bool di2_short = bits & (1<<5);
                bool di3_state = bits & (1<<6);
                bool di3_break = bits & (1<<7);
                bool di3_short = bits & (1<<8);
                bool do1_state = bits & (1<<9);
                bool do2_state = bits & (1<<10);
                bool not_actual = bits & (1<<11);
                if(di1_break) {tree->setGroupValue(i,"di1",Input::BREAK);}
                else if(di1_short) {tree->setGroupValue(i,"di1",Input::SHORT);}
                else if(di1_state) {tree->setGroupValue(i,"di1",Input::ON);}
                else {tree->setGroupValue(i,"di1",Input::OFF);}
                if(di2_break) {tree->setGroupValue(i,"di2",Input::BREAK);}
                else if(di2_short) {tree->setGroupValue(i,"di2",Input::SHORT);}
                else if(di2_state) {tree->setGroupValue(i,"di2",Input::ON);}
                else {tree->setGroupValue(i,"di2",Input::OFF);}
                if(di3_break) {tree->setGroupValue(i,"di3",Input::BREAK);}
                else if(di3_short) {tree->setGroupValue(i,"di3",Input::SHORT);}
                else if(di3_state) {tree->setGroupValue(i,"di3",Input::ON);}
                else {tree->setGroupValue(i,"di3",Input::OFF);}
                tree->setGroupValue(i,"do1",do1_state);
                tree->setGroupValue(i,"do2",do2_state);
                tree->setGroupValue(i,"not_actual",not_actual);
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
}

void MainWindow::stopRecord()
{
    m_audiOutputDevice->stopRecordCmd();
}

void MainWindow::sqlError(const QString &message)
{
    QMessageBox::critical(nullptr, tr("VOIP ДИспетчер"),message);
}

void MainWindow::radioButton_toggled(bool checked)
{
    if(checked) {
        QRadioButton *rb = dynamic_cast<QRadioButton*>(sender());
        if(rb) {
            int id = rb->property("id").toInt();
            udpScanner->setToID(static_cast<quint8>(id));
        }
    }
}

void MainWindow::on_checkBox_clicked()
{
    static QString fName;
    static QStack<QProcess*> procs;
    static QStack<QString> wafFiles;
    static qint64 startTime = 0;
    udpScanner->setSilentMode(ui->checkBox->isChecked());
    if(ui->checkBox->isChecked()) {
        ui->checkBox->setToolTip("режим прослушивания");
        ui->checkBox->setText("РЕЖИМ ПРОСЛУШИВАНИЯ");
        manager->insertMessage("РЕЖИМ ПРОСЛУШИВАНИЯ","сообщение");
    }
    else {
        ui->checkBox->setToolTip("режим передачи голоса");
        ui->checkBox->setText("РЕЖИМ ВЕЩАНИЯ");
        manager->insertMessage("РЕЖИМ ВЕЩАНИЯ","сообщение");
    }
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

        /*connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
               [=](int , QProcess::ExitStatus ){ process->deleteLater(); QFile::remove(inpName);});*/

        dispRecorder = nullptr;
    }
    if(buttonCmd == STOP) {
        if(ui->checkBox->isChecked()) {

        }else {
            dispRecorder = new QAudioRecorder;
            audioSettings.setCodec("audio/pcm");
            audioSettings.setChannelCount(1);
            audioSettings.setBitRate(128*1024);
            audioSettings.setSampleRate(8000);
            audioSettings.setEncodingMode(QMultimedia::AverageBitRateEncoding);

            dispRecorder->setAudioInput(ui->comboBoxInput->currentText());
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
                    on_checkBox_clicked();
                }
            });
        }
    }
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
