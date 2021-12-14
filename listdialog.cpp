#include <QDebug>
#include <QVariant>

#include "listdialog.h"
#include "ui_listdialog.h"

SpeakerListModel::SpeakerListModel(const QList<Speaker> &speakers, QWidget *parent) :
    QAbstractListModel(parent)
{
    this->speakers = speakers;
}

SpeakerListModel::~SpeakerListModel()
{
}

int SpeakerListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return speakers.size();

    if (parent.row() > 0)
        return 0;

    return speakers.size();
}

QVariant SpeakerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() >= speakers.size()) return QVariant();

    if (role == Qt::DisplayRole) {
        return QVariant(speakers.at(index.row()).getVendor() + " - " + speakers.at(index.row()).getModel());
    } else if (role == Qt::UserRole){
        return QVariant::fromValue<Speaker>(speakers.at(index.row()));
    } else {
        return QVariant();
    }
}

void SpeakerListModel::clear()
{
    speakers.clear();
}

ListDialog::ListDialog(const QList<Speaker> &speakers, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListDialog),
    model(nullptr)
{
    ui->setupUi(this);

    setSpeakerItems(speakers);
    connect(this, &ListDialog::accepted, this, &ListDialog::onAccepted);
    connect(this, &ListDialog::rejected, this, &ListDialog::onRejected);
}

ListDialog::~ListDialog()
{
    delete ui;
    if (model != nullptr)
        delete model;
}

void ListDialog::setSpeakerItems(const QList<Speaker> &speakers)
{
    if (model != nullptr) {
        model->clear();
        delete model;
    }

    model = new SpeakerListModel(speakers, this);
    ui->listDialogListView->setModel(model);
}

void ListDialog::clear()
{
    model->clear();
}

void ListDialog::onAccepted()
{
    QString title = model->data(QModelIndex(ui->listDialogListView->currentIndex()), Qt::DisplayRole).toString();
    Speaker s = static_cast<Speaker> (model->data(QModelIndex(ui->listDialogListView->currentIndex()), Qt::UserRole).value<Speaker>());
    emit speakerItemSelected(title, s);
}

void ListDialog::onRejected()
{
    emit speakerItemCancelled();
}
