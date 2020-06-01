#include "bandpassdialog.h"
#include "ui_bandpassdialog.h"

BandpassDialog::BandpassDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandpassDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(this, SIGNAL(rejected()), this, SLOT(onRejected()));
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
