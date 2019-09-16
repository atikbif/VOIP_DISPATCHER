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
    QTreeWidgetItem *gateItem = new QTreeWidgetItem(QStringList()<<"ШЛЮЗ");
    ui->treeWidget->addTopLevelItem(gateItem);
    QTreeWidgetItem *gate_di1Item = new QTreeWidgetItem(QStringList()<<"вход 1 (разреш. с предыд.)"<<"не известно");
    gate_di1Item->setWhatsThis(1,"gate_di1");
    gateItem->addChild(gate_di1Item);
    QTreeWidgetItem *gate_di2Item = new QTreeWidgetItem(QStringList()<<"вход 2 (предпуск.)"<<"не известно");
    gate_di2Item->setWhatsThis(1,"gate_di2");
    gateItem->addChild(gate_di2Item);
    QTreeWidgetItem *gate_di3Item = new QTreeWidgetItem(QStringList()<<"вход 3 (подтвержд. запуска)"<<"не известно");
    gate_di3Item->setWhatsThis(1,"gate_di3");
    gateItem->addChild(gate_di3Item);
    QTreeWidgetItem *gate_do1Item = new QTreeWidgetItem(QStringList()<<"выход 1 (сигнал)"<<"не известно");
    gate_do1Item->setWhatsThis(1,"gate_do1");
    gateItem->addChild(gate_do1Item);
    QTreeWidgetItem *gate_do2Item = new QTreeWidgetItem(QStringList()<<"выход 2 (готовность)"<<"не известно");
    gate_do2Item->setWhatsThis(1,"gate_do2");
    gateItem->addChild(gate_do2Item);
    QTreeWidgetItem *p_cntItem = new QTreeWidgetItem(QStringList()<<"Число реально подключенных точек"<<"не известно");
    p_cntItem->setWhatsThis(1,"pcnt");
    gateItem->addChild(p_cntItem);
    QFile confFile("conf.json");
      if (confFile.open(QIODevice::ReadOnly)) {
        QByteArray saveData = confFile.readAll();
        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject loadOb = loadDoc.object();
        int p_cnt = 0;
        if (loadOb.contains("points cnt")) {
          p_cnt = loadOb["points cnt"].toString().toInt();
        }
        if (loadOb.contains("audio tmr")) {
          audio_tmr = loadOb["audio tmr"].toString().toInt();
          setTimerInterval(60*audio_tmr);
        }
        if (loadOb.contains("ip1")) {
          ip1 = loadOb["ip1"].toString().toInt();
        }
        if (loadOb.contains("ip2")) {
          ip2 = loadOb["ip2"].toString().toInt();
        }
        if (loadOb.contains("ip3")) {
          ip3 = loadOb["ip3"].toString().toInt();
        }
        if (loadOb.contains("ip4")) {
          ip4 = loadOb["ip4"].toString().toInt();
        }
        //if(udpScanner!=nullptr) udpScanner->setIP(QString::number(ip1)+"."+QString::number(ip2)+"."+QString::number(ip3)+"."+QString::number(ip4));
        if (loadOb.contains("points") && loadOb["points"].isArray()) {
          QJsonArray jsPoints = loadOb["points"].toArray();
        point_cnt = p_cnt;
        manager->setPointCnt(point_cnt);
        points.clear();
        for (int i = 0; i < p_cnt; ++i) {
            QJsonObject pointOb = jsPoints[i].toObject();
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<pointOb["name"].toString());
            gateItem->addChild(treeItem);
            QTreeWidgetItem *soundItem = new QTreeWidgetItem(QStringList()<<"Динамики"<<"не известно");
            soundItem->setWhatsThis(1,"p_"+QString::number(i+1)+"_audio");
            treeItem->addChild(soundItem);
            QTreeWidgetItem *powItem = new QTreeWidgetItem(QStringList()<<"Питание"<<"не известно");
            powItem->setWhatsThis(1,"p_"+QString::number(i+1)+"_power");
            treeItem->addChild(powItem);
            QTreeWidgetItem *accItem = new QTreeWidgetItem(QStringList()<<"Аккумулятор"<<"не известно");
            accItem->setWhatsThis(1,"p_"+QString::number(i+1)+"_acc");
            treeItem->addChild(accItem);
            QTreeWidgetItem *di1Item = new QTreeWidgetItem(QStringList()<<"вход 1 (КТВ)"<<"не известно");
            di1Item->setWhatsThis(1,"p_"+QString::number(i+1)+"_di1");
            treeItem->addChild(di1Item);
            QTreeWidgetItem *di2Item = new QTreeWidgetItem(QStringList()<<"вход 2"<<"не известно");
            di2Item->setWhatsThis(1,"p_"+QString::number(i+1)+"_di2");
            treeItem->addChild(di2Item);
            QTreeWidgetItem *do1Item = new QTreeWidgetItem(QStringList()<<"выход 1 (разрешение)"<<"не известно");
            do1Item->setWhatsThis(1,"p_"+QString::number(i+1)+"_do1");
            treeItem->addChild(do1Item);
            QTreeWidgetItem *do2Item = new QTreeWidgetItem(QStringList()<<"выход 2 (предстарт.)"<<"не известно");
            do2Item->setWhatsThis(1,"p_"+QString::number(i+1)+"_do2");
            treeItem->addChild(do2Item);
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
    layout->addStretch(1);
  }
  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->collapseAll();
}

void MainWindow::updateAlarmList(const QStringList &list)
{
    ui->listWidgetAlarm->clear();
    QStringList newList;
    if(linkState==false && buttonCmd==STOP) newList.append("АВАРИЯ (Обрыв Ethernet связи)");
    newList.append(list);
    if(newList.isEmpty()) {
        ui->listWidgetAlarm->addItem("Параметры в норме");
        ui->listWidgetAlarm->item(0)->setBackgroundColor(QColor(0,255,0,150));
    }else {
        ui->listWidgetAlarm->addItems(newList);
        for(int i=0;i<newList.length();i++) {
            if(newList.at(i).toLower().contains("авария")) ui->listWidgetAlarm->item(i)->setBackgroundColor(QColor(255,0,0,150));
            else if(newList.at(i).toLower().contains("предупреждение")) ui->listWidgetAlarm->item(i)->setBackgroundColor(QColor(255,255,0,150));
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
    QString ip = QString::number((quint8)ip1) + ".";
    ip += QString::number((quint8)ip2) + ".";
    ip += QString::number((quint8)ip3) + ".";
    ip += QString::number((quint8)ip4);
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
    QTreeWidgetItem *gateItem = nullptr;
    for(QTreeWidgetItem *item:items) {
        if(item->whatsThis(1)=="pcnt") {
            int cnt = (quint16)((quint8)state.at(0))<<8 | (quint8)state.at(1);
            item->setText(1,QString::number(cnt));
            item->setBackgroundColor(0, QColor(255,255,255,0));
            if(cnt<point_cnt) {
                gate_alarm = true;
                gateItem = item->parent();
                item->setBackgroundColor(0, QColor(255,0,0,150));
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
    }
    manager->insertData(state);
    for(int i=0;i<100;i++) {
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
                QString pow = QString::number((double)((quint8)state.at(32+i*2+1))/10);

                item->setText(1,pow + " В");
            }else if(item->whatsThis(1)==link_acc) {
                item->setText(1,QString::number((double)((quint8)state.at(32+i*2))/10)+" В");
            }else if(item->whatsThis(1)==link_di1) {
                bool alarm = false;
                QString res = "выкл";
                uint16_t byte_num_state = 232 + (16+i*10+1)/8;
                uint16_t byte_num_break = 232 + (16+i*10+2)/8;
                uint16_t byte_num_short = 232 + (16+i*10+3)/8;
                uint8_t bit_num_state = (16+i*10+1)%8;
                uint8_t bit_num_break = (16+i*10+2)%8;
                uint8_t bit_num_short = (16+i*10+3)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                else if(state.at(byte_num_break)&(1<<bit_num_break)) {res = "обрыв";alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ОБРЫВ");}
                else if(state.at(byte_num_short)&(1<<bit_num_short)) {res = "замыкание";alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ЗАМЫКАНИЕ");}
                else {alarm=true;pointAlarms.append("АВАРИЯ ВХОД1(КТВ) - ВЫКЛ");}
                pointItem = item->parent();
                item->setBackgroundColor(0, QColor(255,255,255,0));
                if(alarm) {
                    point_alarm = true;
                    item->setBackgroundColor(0, QColor(255,0,0,150));
                }
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_di2) {
                QString res = "выкл";
                uint16_t byte_num_state = 232 + (16+i*10+4)/8;
                uint16_t byte_num_break = 232 + (16+i*10+5)/8;
                uint16_t byte_num_short = 232 + (16+i*10+6)/8;
                uint8_t bit_num_state = (16+i*10+4)%8;
                uint8_t bit_num_break = (16+i*10+5)%8;
                uint8_t bit_num_short = (16+i*10+6)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                else if(state.at(byte_num_break)&(1<<bit_num_break)) {res = "обрыв";}
                else if(state.at(byte_num_short)&(1<<bit_num_short)) {res = "замыкание";}
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_do1) {
                QString res = "выкл";
                uint16_t byte_num_state = 232 + (16+i*10+7)/8;
                uint8_t bit_num_state = (16+i*10+7)%8;;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_do2) {
                QString res = "выкл";
                uint16_t byte_num_state = 232 + (16+i*10+8)/8;
                uint8_t bit_num_state = (16+i*10+8)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) res = "вкл";
                item->setText(1,res);
            }else if(item->whatsThis(1)==link_audio) {
                bool alarm = true;
                QString res = "не исправны";
                uint16_t byte_num_state = 232 + (16+i*10+0)/8;
                uint8_t bit_num_state = (16+i*10+0)%8;
                uint16_t byte_num_check = 232 + (16+i*10+9)/8;
                uint8_t bit_num_check = (16+i*10+9)%8;
                if(state.at(byte_num_state)&(1<<bit_num_state)) {res = "исправны";alarm=false;}
                if((state.at(byte_num_check)&(1<<bit_num_check))==0) {res = "не проверялись";alarm=false;}
                item->setText(1,res);
                item->setBackgroundColor(0, QColor(255,255,255,0));
                if(alarm) {
                    pointAlarms.append("АВАРИЯ НЕИСПРАВНОСТЬ ДИНАМИКОВ");
                    point_alarm = true;
                    item->setBackgroundColor(0, QColor(255,0,0,150));
                }
            }
        }
        if(pointItem) {
            pointItem->setBackgroundColor(0, QColor(255,255,255,0));
            gateItem = pointItem->parent();
            if(point_alarm) {
                alarmList.append(pointItem->text(0)+":");
                alarmList.append(pointAlarms);
                alarmList.append("");
                gate_alarm = true;
                pointItem->setBackgroundColor(0, QColor(255,0,0,150));
            }
        }

    }
    if(gateItem) {
        gateItem->setBackgroundColor(0, QColor(255,255,255,0));
        if(gate_alarm) {
            gateItem->setBackgroundColor(0, QColor(255,0,0,150));
        }
    }
    updateAlarmList(alarmList);
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
            udpScanner->setToID(id);
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
        QDir dir(QCoreApplication::applicationDirPath());;
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
