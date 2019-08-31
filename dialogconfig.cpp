#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

void DialogConfig::readPoints()
{
  bool readConf = false;

    for (int i = 0; i < 100; i++) {
    ui->tableWidget->insertRow(i);

        QLineEdit *name = new QLineEdit();
        PointState point;
        point.name = name;
        points.push_back(point);

        ui->tableWidget->setCellWidget(i,0,name);
    }

  QFile confFile("conf.json");
  if(confFile.open(QIODevice::ReadOnly)) {
    QByteArray saveData = confFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject loadOb = loadDoc.object();
    if(loadOb.contains("points cnt")) {
      QString cnt = loadOb["points cnt"].toString();
      ui->spinBoxPointCnt->setValue(cnt.toInt());
    }
    if(loadOb.contains("audio tmr")) {
      QString cnt = loadOb["audio tmr"].toString();
      ui->spinBoxCheckAudioTmr->setValue(cnt.toInt());
    }
    if(loadOb.contains("points") && loadOb["points"].isArray()) {
      QJsonArray jsPoints = loadOb["points"].toArray();
      for (int i = 0; i < jsPoints.size(); ++i) {
        QJsonObject pointOb = jsPoints[i].toObject();
        bool checkPoint = true;
        if(!pointOb.contains("address") || !pointOb["address"].isDouble()) checkPoint = false;
        if(!pointOb.contains("name") || !pointOb["name"].isString()) checkPoint = false;
        if(!pointOb.contains("on") || !pointOb["on"].isBool()) checkPoint = false;
        if(checkPoint) {
          int num = pointOb["address"].toInt()-1;
          if(num>=0 && num<(int)points.size()) {
            points.at((std::size_t)num).name->setText(pointOb["name"].toString());
            readConf = true;
          }
        }
      }
    }
  }
  if(readConf==false) {
    for(int i=0;i<3;i++) {
      points.at((std::size_t)i).name->setText("ТОЧКА " + QString::number(i+1));
    }
  }
}

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(1); // Указываем число колонок
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "имя");
    // Растягиваем последнюю колонку на всё доступное пространство
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    readPoints();
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
    QJsonArray pointsArray;
    int addr = 1;
    for(PointState point:points) {
      QJsonObject p({{"name",point.name->text()},{"address",addr++}});
      pointsArray.append(p);
    }
    confObject["version"] = "1.0";
    confObject["points cnt"] = QString::number(ui->spinBoxPointCnt->value());
    confObject["audio tmr"] = QString::number(ui->spinBoxCheckAudioTmr->value());
    confObject["points"] = pointsArray;
    QJsonDocument confDoc(confObject);
    confFile.write(confDoc.toJson());
    confFile.close();
  }
  QDialog::accept();
}
