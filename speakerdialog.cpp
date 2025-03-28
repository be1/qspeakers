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

    connect(this, &SpeakerDialog::accepted, this, &SpeakerDialog::onSpeakerAccepted);
    connect(this, &SpeakerDialog::rejected, this, &SpeakerDialog::onSpeakerRejected);
    connect(ui->vendorLineEdit, &QLineEdit::textChanged, this, &SpeakerDialog::onVendorTextChanged);
    connect(ui->modelLineEdit, &QLineEdit::textChanged, this, &SpeakerDialog::onModelTextChanged);
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
    ui->vcSpinBox->setValue(edit.getVc());

    oldVendor = edit.getVendor();
    oldModel = edit.getModel();

    ui->vendorLineEdit->setText(oldVendor);
    ui->modelLineEdit->setText(oldModel);

    connect(this, &SpeakerDialog::accepted, this, &SpeakerDialog::onSpeakerAccepted);
    connect(this, &SpeakerDialog::rejected, this, &SpeakerDialog::onSpeakerRejected);
    connect(ui->vendorLineEdit, &QLineEdit::textChanged, this, &SpeakerDialog::onVendorTextChanged);
    connect(ui->modelLineEdit, &QLineEdit::textChanged, this, &SpeakerDialog::onModelTextChanged);
}

SpeakerDialog::~SpeakerDialog()
{
    delete ui;
}

void SpeakerDialog::onVendorTextChanged(QString text)
{
    if (text.isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else if (!ui->modelLineEdit->text().isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SpeakerDialog::onModelTextChanged(QString text)
{
    if (text.isEmpty())
        ui->speakerButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else if (!ui->vendorLineEdit->text().isEmpty())
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
    spk.setVc(ui->vcSpinBox->value());

    spk.computeEmptyParams();


    if (oldVendor.isEmpty() || oldModel.isEmpty()) {
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
