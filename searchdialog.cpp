#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);

    QStringList params = QString("Fs,Qts,Vas,Dia,Z,Sd,Re,Xmax,Le,Qms,Qes,Spl,Pe,BL").split(',');

    ui->searchComboBox->addItems(params);

    connect(this, &SearchDialog::accepted, this, &SearchDialog::onAccepted);
    connect(this, &SearchDialog::rejected, this, &SearchDialog::onRejected);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::onAccepted()
{
    QString param = ui->searchComboBox->currentText();
    double min = ui->searchMinDoubleSpinBox->value();
    double max = ui->searchMaxdoubleSpinBox->value();
    QString p = param.toLower();

    emit searchRequested(p, min, max);
}

void SearchDialog::onRejected()
{
    emit searchCancelled();
}

