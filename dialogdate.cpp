#include "dialogdate.h"
#include "ui_dialogdate.h"

DialogDate::DialogDate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDate)
{
    ui->setupUi(this);
}

DialogDate::~DialogDate()
{
    delete ui;
}

void DialogDate::setDate(const QDate &date)
{
    ui->calendarWidget->setSelectedDate(date);
}

QDate DialogDate::getDate()
{
    return ui->calendarWidget->selectedDate();
}
