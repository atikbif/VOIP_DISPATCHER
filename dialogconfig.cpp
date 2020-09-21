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
  prConf->readConfig();

    for (int i = 0; i < GateState::maxPointQuantity; i++) {
        ui->tablePointWidget->insertRow(i);
        QLineEdit *name = new QLineEdit();
        name->setEnabled(false);
        ui->tablePointWidget->setCellWidget(i,0,name);
        connect(name,&QLineEdit::textChanged,[this](const QString &text){
            if(updFlag==false) {
                int gateNum = ui->tableGateWidget->currentRow();
                int pointNum = ui->tablePointWidget->currentRow();
                if(gateNum>=0 && pointNum>=0 && (gateNum < static_cast<int>(gates.size())) && (pointNum< static_cast<int>(gates.at(static_cast<std::size_t>(gateNum)).points.size()))) {
                    gates[static_cast<std::size_t>(gateNum)].points[static_cast<std::size_t>(pointNum)]=text;
                }
            }
        });
    }

    curGateCnt = static_cast<int>(prConf->gates.size());

    for (int i = 0; i < ProjectConfig::maxGateQuantity; i++) {
        ui->tableGateWidget->insertRow(i);
        QLineEdit *name = new QLineEdit();
        name->setEnabled(false);
        connect(name,&QLineEdit::textChanged,[this](const QString &text){
            if(updFlag==false) {
                int gateNum = ui->tableGateWidget->currentRow();
                if(gateNum>=0 && (gateNum < static_cast<int>(gates.size())) ) gates[static_cast<std::size_t>(gateNum)].name = text;
            }
        });
        ui->tableGateWidget->setCellWidget(i,0,name);
        GateState gate;
        gate.name = "";
        for(int j=0;j<ui->tablePointWidget->rowCount();j++) {
            gate.points.push_back("");
        }
        gates.push_back(gate);
    }
    ui->spinBoxIP1->setValue(prConf->ip1.toInt());
    ui->spinBoxIP2->setValue(prConf->ip2.toInt());
    ui->spinBoxIP3->setValue(prConf->ip3.toInt());
    ui->spinBoxIP4->setValue(prConf->ip4.toInt());
    ui->spinBoxCheckAudioTmr->setValue(prConf->tmr.toInt());
    ui->spinBoxGateCnt->setValue(static_cast<int>(prConf->gates.size()));
    gates = prConf->gates;
    for(int i=0;i<static_cast<int>(gates.size());i++) {
        QLineEdit* g= dynamic_cast<QLineEdit*>(ui->tableGateWidget->cellWidget(i,0));
        if(g) {
            updFlag=true;
            g->setText(gates.at(static_cast<std::size_t>(i)).name);
            g->setEnabled(true);
            updFlag=false;
        }
    }
    ui->tableGateWidget->setCurrentCell(0,0);
    ui->tablePointWidget->setCurrentCell(0,0);
}

DialogConfig::DialogConfig(ProjectConfig *prConf, QWidget *parent) :
    QDialog(parent), prConf(prConf),
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
    while(static_cast<int>(gates.size()) > curGateCnt) {
        gates.pop_back();
    }
    prConf->gates = gates;
    prConf->ip1 = QString::number(ui->spinBoxIP1->value());
    prConf->ip2 = QString::number(ui->spinBoxIP2->value());
    prConf->ip3 = QString::number(ui->spinBoxIP3->value());
    prConf->ip4 = QString::number(ui->spinBoxIP4->value());
    prConf->tmr = QString::number(ui->spinBoxCheckAudioTmr->value());
    prConf->writeConfig();
    QDialog::accept();
}

void DialogConfig::on_tableGateWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    updFlag=true;
    if(currentRow>=0 && currentRow < static_cast<int>(gates.size())) {
        GateState gate = gates[static_cast<std::vector<QString>::size_type>(currentRow)];
        int length = static_cast<int>(gate.points.size());
        ui->spinBoxPointCnt->setValue(length);
        for(int i=0;i<GateState::maxPointQuantity;i++) {
            QWidget* w = ui->tablePointWidget->cellWidget(i,0);
            QLineEdit *p = dynamic_cast<QLineEdit*>(w);
            if(p) {
                if(i<length) {
                    QString txt = gate.points.at(static_cast<std::size_t>(i));
                    p->setText(txt);
                    if(currentRow<curGateCnt) p->setEnabled(true);
                }else{
                    p->setText("");
                    p->setEnabled(false);
                }
            }
        }
    }
    updFlag=false;
}

void DialogConfig::on_spinBoxPointCnt_valueChanged(int arg1)
{
    int gateNum = ui->tableGateWidget->currentRow();
    int cnt = static_cast<int>(gates.size());
    if(gateNum<cnt) {
        int pCnt = static_cast<int>(gates.at(static_cast<std::size_t>(gateNum)).points.size());
        if(!updFlag) {
            while(arg1>pCnt) {
                gates[static_cast<std::size_t>(gateNum)].points.push_back("");
                pCnt = static_cast<int>(gates.at(static_cast<std::size_t>(gateNum)).points.size());
            }
            if(arg1<pCnt) {
                while(arg1!=pCnt) {
                    gates[static_cast<std::size_t>(gateNum)].points.pop_back();
                    pCnt = static_cast<int>(gates.at(static_cast<std::size_t>(gateNum)).points.size());
                }
            }
        }
        updFlag = true;
        for(int i=0;i<GateState::maxPointQuantity;i++) {
            QWidget* w = ui->tablePointWidget->cellWidget(i,0);
            QLineEdit *p = dynamic_cast<QLineEdit*>(w);
            if(p) {
                if(i<arg1) p->setEnabled(true);
                else p->setEnabled(false);
                if(i<pCnt) p->setText(gates.at(static_cast<std::size_t>(gateNum)).points.at(static_cast<std::size_t>(i)));
                else p->setText("");
            }
        }
        updFlag = false;

    }
}

void DialogConfig::on_spinBoxGateCnt_valueChanged(int arg1)
{
    int cnt = static_cast<int>(gates.size());
    for(int i=0;i<ProjectConfig::maxGateQuantity;i++) {
        QWidget* w = ui->tableGateWidget->cellWidget(i,0);
        QLineEdit *p = dynamic_cast<QLineEdit*>(w);
        if(p) {
            if(i<arg1) p->setEnabled(true);
            else p->setEnabled(false);
        }
    }

    while(arg1>cnt) {
        GateState gate;
        gate.name="";
        gate.points.push_back("Точка 1");
        gates.push_back(gate);
        cnt = static_cast<int>(gates.size());
    }
    curGateCnt = arg1;
}
