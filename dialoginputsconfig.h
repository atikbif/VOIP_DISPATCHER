#ifndef DIALOGINPUTSCONFIG_H
#define DIALOGINPUTSCONFIG_H

#include <QDialog>
#include <vector>
#include <QString>

struct PointConf {
    QString name;
    bool inp1En;
    bool inp2En;
    quint8 in1Filter;
    quint8 in2Filter;
};

struct GateConf {
    QString name;
    std::vector<PointConf> points;
};

namespace Ui {
class DialogInputsConfig;
}

class DialogInputsConfig : public QDialog
{
    Q_OBJECT
    std::vector<GateConf> groups;
public:
    explicit DialogInputsConfig(QWidget *parent = nullptr);
    void addGateConf(const GateConf &conf);
    ~DialogInputsConfig();

private slots:
    void on_comboBoxGroups_currentIndexChanged(int index);

    void on_comboBoxPoints_currentIndexChanged(int index);

private:
    Ui::DialogInputsConfig *ui;
};

#endif // DIALOGINPUTSCONFIG_H
