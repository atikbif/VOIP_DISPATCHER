#ifndef DIALOGDATE_H
#define DIALOGDATE_H

#include <QDialog>
#include <QDate>

namespace Ui {
class DialogDate;
}

class DialogDate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDate(QWidget *parent = nullptr);
    ~DialogDate();
    void setDate(const QDate &date);
    QDate getDate();

private:
    Ui::DialogDate *ui;
};

#endif // DIALOGDATE_H
