#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog)
{
    ui->setupUi(this);
}

PropertiesDialog::~PropertiesDialog()
{
    delete ui;
}

void PropertiesDialog::setTitle(const QString &title)
{
    ui->titleLineEdit->setText(title);
}

void PropertiesDialog::setNote(const QString &note)
{
    ui->noteTextEdit->setPlainText(note);
}

QString PropertiesDialog::getTitle()
{
    return ui->titleLineEdit->text();
}

QString PropertiesDialog::getNote()
{
    return ui->noteTextEdit->toPlainText();
}
