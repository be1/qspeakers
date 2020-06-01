#include <QPushButton>
#include <QDebug>
#include "speakerdialog.h"
#include "ui_speakerdialog.h"
#include "speaker.h"
#include "speakerdb.h"

SpeakerDialog::SpeakerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpeakerDialog)
{
    ui->setupUi(this);

    /* do not enable 'ok' since model and vendor are empty */
    ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(this, SIGNAL(accepted()), this, SLOT(onSpeakerAccepted()));
    connect(this, SIGNAL(rejected()), this, SLOT(onSpeakerRejected()));
    connect(ui->vendorLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onVendorTextChanged(QString)));
    connect(ui->modelLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onModelTextChanged(QString)));
}

SpeakerDialog::SpeakerDialog(const Speaker &edit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpeakerDialog)
{
    ui->setupUi(this);

    ui->fsDoubleSpinBox->setValue(edit.getFs());
    ui->qtsDoubleSpinBox->setValue(edit.getQts());
    ui->vasDoubleSpinBox->setValue(edit.getVas());
    ui->zDoubleSpinBox->setValue(edit.getZ());
    ui->diaDoubleSpinBox->setValue(edit.getDia());
    ui->reDoubleSpinBox->setValue(edit.getRe());
    ui->sdDoubleSpinBox->setValue(edit.getSd());
    ui->xmaxDoubleSpinBox->setValue(edit.getXmax());
    ui->leDoubleSpinBox->setValue(edit.getLe());
    ui->qmsDoubleSpinBox->setValue(edit.getQms());
    ui->qesDoubleSpinBox->setValue(edit.getQes());
    ui->splDoubleSpinBox->setValue(edit.getSpl());
    ui->peDoubleSpinBox->setValue(edit.getPe());
    ui->blDoubleSpinBox->setValue(edit.getBL());

    oldVendor = edit.getVendor();
    oldModel = edit.getModel();

    ui->vendorLineEdit->setText(oldVendor);
    ui->modelLineEdit->setText(oldModel);

    connect(this, SIGNAL(accepted()), this, SLOT(onSpeakerAccepted()));
    connect(this, SIGNAL(rejected()), this, SLOT(onSpeakerRejected()));
    connect(ui->vendorLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onVendorTextChanged(QString)));
    connect(ui->modelLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onModelTextChanged(QString)));
}

SpeakerDialog::~SpeakerDialog()
{
    delete ui;
}

void SpeakerDialog::onVendorTextChanged(QString text)
{
    if (text.isNull() || text.isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else if (!ui->modelLineEdit->text().isNull() && !ui->modelLineEdit->text().isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SpeakerDialog::onModelTextChanged(QString text)
{
    if (text.isNull() || text.isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else if (!ui->vendorLineEdit->text().isNull() && !ui->vendorLineEdit->text().isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SpeakerDialog::onSpeakerAccepted()
{
    Speaker spk;
    QString vendor = ui->vendorLineEdit->text();
    QString model = ui->modelLineEdit->text();

    spk.setVendor(vendor);
    spk.setModel(model);

    spk.setFs(ui->fsDoubleSpinBox->value());
    spk.setVas(ui->vasDoubleSpinBox->value());
    spk.setQts(ui->qtsDoubleSpinBox->value());
    spk.setZ(ui->zDoubleSpinBox->value());
    spk.setDia(ui->diaDoubleSpinBox->value());
    spk.setRe(ui->reDoubleSpinBox->value());
    spk.setSd(ui->sdDoubleSpinBox->value());
    spk.setXmax(ui->xmaxDoubleSpinBox->value());
    spk.setLe(ui->leDoubleSpinBox->value());
    spk.setQms(ui->qmsDoubleSpinBox->value());
    spk.setQes(ui->qesDoubleSpinBox->value());
    spk.setSpl(ui->splDoubleSpinBox->value());
    spk.setPe(ui->peDoubleSpinBox->value());
    spk.setBL(ui->blDoubleSpinBox->value());


    if (oldVendor.isNull() || oldModel.isNull()) {
        SpeakerDb::insertOrReplace(vendor, model, spk);
    } else {
        SpeakerDb::insertOrReplace(oldVendor, oldModel, spk);
    }
    emit speakerInserted(spk);
}

void SpeakerDialog::onSpeakerRejected()
{
    emit speakerCancelled();
}
