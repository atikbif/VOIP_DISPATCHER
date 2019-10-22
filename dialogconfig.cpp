#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

void DialogConfig::readData()
{
  bool readConf = false;

    for (int i = 0; i < 100; i++) {
        ui->tablePointWidget->insertRow(i);
        QLineEdit *name = new QLineEdit();
        ui->tablePointWidget->setCellWidget(i,0,name);
        connect(name,&QLineEdit::textChanged,[this](const QString &text){
            if(updFlag==false) {
                int gateNum = ui->tableGateWidget->currentRow();
                int pointNum = ui->tablePointWidget->currentRow();
                if(gateNum>=0 && pointNum>=0) gates[gateNum].points[pointNum]=text;
            }
        });
    }

    for (int i = 0; i < 32; i++) {
        ui->tableGateWidget->insertRow(i);
        QLineEdit *name = new QLineEdit();
        connect(name,&QLineEdit::textChanged,[this](const QString &text){
            if(updFlag==false) {
                int gateNum = ui->tableGateWidget->currentRow();
                gates[gateNum].name = text;
            }
        });
        ui->tableGateWidget->setCellWidget(i,0,name);
        GateState gate;
        gate.name = "";
        for(int j=0;j<ui->tablePointWidget->rowCount();j++) {
            gate.points.push_back("");
        }
        gate.count=0;
        gates.push_back(gate);
    }
    QFile confFile("conf.json");
    if(confFile.open(QIODevice::ReadOnly)) {
        QByteArray saveData = confFile.readAll();
        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject loadOb = loadDoc.object();
        if(loadOb.contains("ip1")) {
          QString cnt = loadOb["ip1"].toString();
          ui->spinBoxIP1->setValue(cnt.toInt());
        }
        if(loadOb.contains("ip2")) {
          QString cnt = loadOb["ip2"].toString();
          ui->spinBoxIP2->setValue(cnt.toInt());
        }
        if(loadOb.contains("ip3")) {
          QString cnt = loadOb["ip3"].toString();
          ui->spinBoxIP3->setValue(cnt.toInt());
        }
        if(loadOb.contains("ip4")) {
          QString cnt = loadOb["ip4"].toString();
          ui->spinBoxIP4->setValue(cnt.toInt());
        }
        if(loadOb.contains("audio tmr")) {
          QString cnt = loadOb["audio tmr"].toString();
          ui->spinBoxCheckAudioTmr->setValue(cnt.toInt());
        }
        if(loadOb.contains("gate cnt")) {
          QString cnt = loadOb["gate cnt"].toString();
          ui->spinBoxGateCnt->setValue(cnt.toInt());
        }
        if(loadOb.contains("gates") && loadOb["gates"].isArray()) {
          QJsonArray jsGates = loadOb["gates"].toArray();
          int gate_length = jsGates.size();
          if(gate_length>gates.size()) gate_length=gates.size();

          for (int i = 0; i < gate_length; ++i) {
            QJsonObject gateOb = jsGates[i].toObject();
            if(gateOb.contains("name")) {
              QString name = gateOb["name"].toString();
              gates[i].name = name;
              QLineEdit* g= dynamic_cast<QLineEdit*>(ui->tableGateWidget->cellWidget(i,0));
              if(g) {updFlag=true;g->setText(name);updFlag=false;}
            }
            if(gateOb.contains("cnt")) {
                int cnt = gateOb["cnt"].toInt();
                gates[i].count = cnt;
            }
            if(gateOb.contains("points")&& gateOb["points"].isArray()) {
                QJsonArray jsPoints = gateOb["points"].toArray();
                int points_length = jsPoints.size();
                if(points_length>gates[i].points.size()) gate_length=gates[i].points.size();
                for(int j=0;j<points_length;j++) {
                    QJsonObject pointOb = jsPoints[j].toObject();
                    if(pointOb.contains("name")) {
                        QString name = pointOb["name"].toString();
                        gates[i].points[j]=name;
                        readConf = true;
                    }
                }
            }
          }
      }
  }
  if(readConf==false) {
      gates[0].name = "ГРУППА 1";
      QLineEdit* g= dynamic_cast<QLineEdit*>(ui->tableGateWidget->cellWidget(0,0));
      if(g) {updFlag=true;g->setText(gates[0].name);updFlag=false;}
      gates[0].count = 3;

      for(int i=0;i<gates[0].count;i++) {
          gates[0].points[i]="Точка "+QString::number(i+1);
      }
      gates[1].name = "ГРУППА 2";
      g= dynamic_cast<QLineEdit*>(ui->tableGateWidget->cellWidget(1,0));
      if(g) {updFlag=true;g->setText(gates[1].name);updFlag=false;}
      gates[1].count = 2;
      for(int i=0;i<gates[1].count;i++) {
          gates[1].points[i]="Точка "+QString::number(i+1);
      }
      ui->spinBoxGateCnt->setValue(2);
      ui->spinBoxIP1->setValue(192);
      ui->spinBoxIP2->setValue(168);
      ui->spinBoxIP3->setValue(1);
      ui->spinBoxIP4->setValue(2);
  }
  ui->tableGateWidget->setCurrentCell(0,0);
  ui->tablePointWidget->setCurrentCell(0,0);
}

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);
    ui->tablePointWidget->clear();
    ui->tablePointWidget->setColumnCount(1); // Указываем число колонок
    ui->tablePointWidget->setShowGrid(true);
    ui->tablePointWidget->setHorizontalHeaderLabels(QStringList() << "Точки");
    // Растягиваем последнюю колонку на всё доступное пространство
    ui->tablePointWidget->horizontalHeader()->setStretchLastSection(true);


    ui->tableGateWidget->clear();
    ui->tableGateWidget->setColumnCount(1); // Указываем число колонок
    ui->tableGateWidget->setShowGrid(true);
    ui->tableGateWidget->setHorizontalHeaderLabels(QStringList() << "Группы");
    // Растягиваем последнюю колонку на всё доступное пространство
    ui->tableGateWidget->horizontalHeader()->setStretchLastSection(true);

    readData();
}

DialogConfig::~DialogConfig()
{
    delete ui;
}

void DialogConfig::accept()
{
  QFile confFile("conf.json");
  if(confFile.open(QIODevice::WriteOnly)) {
    QJsonObject confObject;
    QJsonArray gateArray;
    for(int i=0;i<gates.size();i++) {
        QJsonObject g;
        g["name"] = gates.at(i).name;
        g["cnt"] = gates.at(i).count;
        QJsonArray pointArray;
        for(int j=0;j<gates.at(i).points.size();j++) {
            QJsonObject p;
            p["name"]=gates.at(i).points.at(j);
            pointArray.append(p);
        }
        g["points"] = pointArray;
        gateArray.append(g);
    }
    confObject["version"] = "1.0";
    confObject["gate cnt"] = QString::number(ui->spinBoxGateCnt->value());
    confObject["audio tmr"] = QString::number(ui->spinBoxCheckAudioTmr->value());
    confObject["gates"] = gateArray;
    confObject["ip1"] = QString::number(ui->spinBoxIP1->value());
    confObject["ip2"] = QString::number(ui->spinBoxIP2->value());
    confObject["ip3"] = QString::number(ui->spinBoxIP3->value());
    confObject["ip4"] = QString::number(ui->spinBoxIP4->value());
    QJsonDocument confDoc(confObject);
    confFile.write(confDoc.toJson());
    confFile.close();
  }
  QDialog::accept();
}

void DialogConfig::on_tableGateWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    updFlag=true;
    GateState gate = gates[static_cast<std::vector<QString>::size_type>(currentRow)];
    ui->spinBoxPointCnt->setValue(gate.count);
    auto length = gate.points.size();
    for(int i=0;i<static_cast<int>(length);i++) {
        QString txt = gate.points.at(i);
        QWidget* w = ui->tablePointWidget->cellWidget(i,0);
        QLineEdit *p = dynamic_cast<QLineEdit*>(w);
        if(p) {p->setText(txt);}
    }
    updFlag=false;
}

void DialogConfig::on_spinBoxPointCnt_valueChanged(int arg1)
{
    int gateNum = ui->tableGateWidget->currentRow();
    gates[gateNum].count=arg1;
}
