#ifndef DIALOGVOLUMECONFIG_H
#define DIALOGVOLUMECONFIG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class DialogVolumeConfig;
}

class DialogVolumeConfig : public QDialog
{
    Q_OBJECT
    QVector<QStringList> points;
    QVector<QStringList> volume;
public:
    explicit DialogVolumeConfig(QWidget *parent = nullptr);
    ~DialogVolumeConfig();
    void addGroups(QStringList groups);
    void addPoints(QStringList points);
    void addVolume(QStringList volume);
    int getCurrentGroup() const;
    int getCurrentPoint() const;
    int getCurrentVolume() const;

private slots:
    void on_comboBoxGroup_currentIndexChanged(int index);

    void on_comboBoxPoint_currentIndexChanged(int index);

private:
    Ui::DialogVolumeConfig *ui;
};

#endif // DIALOGVOLUMECONFIG_H
