#include "dialoginputsconfig.h"
#include "ui_dialoginputsconfig.h"

DialogInputsConfig::DialogInputsConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInputsConfig)
{
    ui->setupUi(this);
}

void DialogInputsConfig::addGateConf(const GateConf &conf)
{
    groups.push_back(conf);
    ui->comboBoxGroups->addItem(conf.name);
    ui->comboBoxGroups->setCurrentIndex(0);
}

DialogInputsConfig::~DialogInputsConfig()
{
    delete ui;
}

void DialogInputsConfig::on_comboBoxGroups_currentIndexChanged(int index)
{
    if(index>=0 && (static_cast<std::size_t>(index) < groups.size())) {
        GateConf conf=groups.at(static_cast<std::size_t>(index));
        ui->comboBoxPoints->clear();
        for(const PointConf &p:conf.points) {
            ui->comboBoxPoints->addItem(p.name);
        }
        if(conf.points.size()==0) {
            ui->checkBoxDI1->setEnabled(false);
            ui->checkBoxDI2->setEnabled(false);
            ui->comboBoxDI1Filter->setEnabled(false);
            ui->comboBoxDI2Filter->setEnabled(false);
        }else{
            on_comboBoxPoints_currentIndexChanged(0);
        }
    }

}


void DialogInputsConfig::on_comboBoxPoints_currentIndexChanged(int index)
{
    int grIndex = ui->comboBoxGroups->currentIndex();
    if(grIndex>=0 && (static_cast<std::size_t>(grIndex) < groups.size())) {
        GateConf conf=groups.at(static_cast<std::size_t>(grIndex));
        if(index>=0 && (static_cast<std::size_t>(index) < conf.points.size())) {
            ui->checkBoxDI1->setEnabled(true);
            ui->checkBoxDI2->setEnabled(true);
            ui->comboBoxDI1Filter->setEnabled(true);
            ui->comboBoxDI2Filter->setEnabled(true);
            if(conf.points.at(static_cast<std::size_t>(index)).inp1En) ui->checkBoxDI1->setChecked(true); else ui->checkBoxDI1->setChecked(false);
            if(conf.points.at(static_cast<std::size_t>(index)).inp2En) ui->checkBoxDI2->setChecked(true); else ui->checkBoxDI2->setChecked(false);
            int filter1 = conf.points.at(static_cast<std::size_t>(index)).in1Filter;
            int filter2 = conf.points.at(static_cast<std::size_t>(index)).in2Filter;
            if(filter1>=0 && filter1<ui->comboBoxDI1Filter->count()) ui->comboBoxDI1Filter->setCurrentIndex(filter1);
            if(filter2>=0 && filter2<ui->comboBoxDI2Filter->count()) ui->comboBoxDI2Filter->setCurrentIndex(filter2);
        }
    }
}
