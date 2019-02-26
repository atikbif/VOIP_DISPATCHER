#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>
#include <vector>
#include <QLineEdit>
#include <QCheckBox>

namespace Ui {
class DialogConfig;
}

struct PointState{
  QLineEdit* name;
  QCheckBox* acivate;
};

class DialogConfig : public QDialog
{
    Q_OBJECT
    std::vector<PointState> points;
    void readPoints();
public:
    explicit DialogConfig(QWidget *parent = nullptr);
    ~DialogConfig();

private slots:
    void on_pushButtonClearAll_clicked();

void on_pushButtonSetAll_clicked();

private:
    Ui::DialogConfig *ui;

    // QDialog interface
public slots:
    void accept();
};

#endif // DIALOGCONFIG_H
