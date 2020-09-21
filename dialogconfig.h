#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>
#include <vector>
#include <QLineEdit>
#include <QCheckBox>
#include "projectconfig.h"

namespace Ui {
class DialogConfig;
}

//struct GateState{
//    QString name;
//    std::vector<QString> points;
//    int count;
//};



class DialogConfig : public QDialog
{
    Q_OBJECT
    std::vector<GateState> gates;
    ProjectConfig *prConf;
    bool updFlag=false;
    void readData();
    int curGateCnt = 0;
public:
    explicit DialogConfig(ProjectConfig *prConf, QWidget *parent = nullptr);
    ~DialogConfig();

private slots:

    void on_tableGateWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_spinBoxPointCnt_valueChanged(int arg1);

    void on_spinBoxGateCnt_valueChanged(int arg1);

private:
    Ui::DialogConfig *ui;

    // QDialog interface
public slots:
    void accept();
};

#endif // DIALOGCONFIG_H
