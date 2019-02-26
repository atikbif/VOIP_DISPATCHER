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

    QWidget *checkBoxWidget = new QWidget();
        QCheckBox *checkBox = new QCheckBox();      // We declare and initialize the checkbox
        QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); // create a layer with reference to the widget
        layoutCheckBox->addWidget(checkBox);            // Set the checkbox in the layer
        layoutCheckBox->setAlignment(Qt::AlignCenter);  // Center the checkbox
        layoutCheckBox->setContentsMargins(0,0,0,0);    // Set the zero padding


        QLineEdit *name = new QLineEdit();
        PointState point;
        point.name = name;
        point.acivate = checkBox;
        points.push_back(point);

        ui->tableWidget->setCellWidget(i,1,name);
        ui->tableWidget->setCellWidget(i,0,checkBoxWidget);
    }

  QFile confFile("conf.json");
  if(confFile.open(QIODevice::ReadOnly)) {
    QByteArray saveData = confFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject loadOb = loadDoc.object();
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
          if(num>=0 && num<points.size()) {
            points.at(num).name->setText(pointOb["name"].toString());
            points.at(num).acivate->setChecked(pointOb["on"].toBool());
            readConf = true;
          }
        }
      }
    }
  }
  if(readConf==false) {
    for(int i=0;i<3;i++) {
      points.at(i).name->setText("ТОЧКА " + QString::number(i+1));
      points.at(i).acivate->setChecked(true);
    }
  }

}

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(2); // Указываем число колонок
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "разрешение" << "имя");
    // Растягиваем последнюю колонку на всё доступное пространство
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    readPoints();
}

DialogConfig::~DialogConfig()
{
    delete ui;
}

void DialogConfig::on_pushButtonClearAll_clicked()
{
  for (PointState point : points) {
    point.acivate->setChecked(false);
  }
}

void DialogConfig::on_pushButtonSetAll_clicked()
{
  for(PointState point:points) {
    point.acivate->setChecked(true);
  }
}

void DialogConfig::accept()
{
  QFile confFile("conf.json");
  if(confFile.open(QIODevice::WriteOnly)) {
    QJsonObject confObject;
    QJsonArray pointsArray;
    int addr = 1;
    for(PointState point:points) {
      QJsonObject p({{"name",point.name->text()},{"on",point.acivate->isChecked()},{"address",addr++}});
      pointsArray.append(p);
    }
    confObject["version"] = "1.0";
    confObject["points"] = pointsArray;
    QJsonDocument confDoc(confObject);
    confFile.write(confDoc.toJson());
    confFile.close();
  }
  QDialog::accept();
}
