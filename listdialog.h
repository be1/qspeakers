#ifndef LISTDIALOG_H
#define LISTDIALOG_H

#include <QDialog>
#include <QAbstractListModel>

#include "speaker.h"

class SpeakerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SpeakerListModel(const QList<Speaker> &speakers, QWidget *parent = 0);
    ~SpeakerListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    void clear(void);

private:
    QList<Speaker> speakers;
};

namespace Ui {
class ListDialog;
}

class ListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ListDialog(const QList<Speaker>& speakers, QWidget *parent = 0);
    ~ListDialog();

    void setSpeakerItems(const QList<Speaker> &speakers);
    void clear(void);

signals:
    void speakerItemSelected(QString title, const Speaker &speaker);
    void speakerItemCancelled();

private slots:
    void onAccepted();
    void onRejected();

private:
    Ui::ListDialog *ui;
    SpeakerListModel *model;
};

#endif // LISTDIALOG_H
