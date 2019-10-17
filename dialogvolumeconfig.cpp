#include "dialogvolumeconfig.h"
#include "ui_dialogvolumeconfig.h"
#include <QtDebug>

DialogVolumeConfig::DialogVolumeConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVolumeConfig)
{
    ui->setupUi(this);
}

DialogVolumeConfig::~DialogVolumeConfig()
{
    delete ui;
}

void DialogVolumeConfig::addGroups(QStringList groups)
{
    ui->comboBoxGroup->clear();
    ui->comboBoxGroup->addItems(groups);
}

void DialogVolumeConfig::addPoints(QStringList points)
{
    this->points.push_back(points);
}

void DialogVolumeConfig::addVolume(QStringList volume)
{
    this->volume.push_back(volume);
}

int DialogVolumeConfig::getCurrentGroup() const
{
    return ui->comboBoxGroup->currentIndex();
}

int DialogVolumeConfig::getCurrentPoint() const
{
    return ui->comboBoxPoint->currentIndex();
}

int DialogVolumeConfig::getCurrentVolume() const
{
    if(ui->comboBoxVolume->currentText()=="максимум") return 0;
    else if(ui->comboBoxVolume->currentText()=="1/2") return 1;
    else if(ui->comboBoxVolume->currentText()=="1/4") return 2;
    else if(ui->comboBoxVolume->currentText()=="1/8") return 3;
    return -1;
}

void DialogVolumeConfig::on_comboBoxGroup_currentIndexChanged(int index)
{
    if(index>=0 && index<points.size()) {
        ui->comboBoxPoint->clear();
        ui->comboBoxPoint->addItems(points.at(index));
    }
}


void DialogVolumeConfig::on_comboBoxPoint_currentIndexChanged(int index)
{
    if(index>=0 && index<volume.at(ui->comboBoxGroup->currentIndex()).size())
    {
        ui->comboBoxVolume->setCurrentText(volume.at(ui->comboBoxGroup->currentIndex()).at(index));
    }
}
