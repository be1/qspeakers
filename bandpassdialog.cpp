#include "bandpassdialog.h"
#include "ui_bandpassdialog.h"

BandpassDialog::BandpassDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandpassDialog)
{
    ui->setupUi(this);

    connect(this, &BandpassDialog::accepted, this, &BandpassDialog::onAccepted);
    connect(this, &BandpassDialog::rejected, this, &BandpassDialog::onRejected);
}

BandpassDialog::~BandpassDialog()
{
    delete ui;
}

void BandpassDialog::onAccepted()
{
    double s, pa;
    s = ui->rippleDoubleSpinBox->value();
    pa = ui->gainSpinBox->value();
    emit optimizeRequested(s, pa);
}

void BandpassDialog::onRejected()
{
    emit optimizeCancelled();
}
