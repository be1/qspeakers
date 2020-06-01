#ifndef SPEAKERDIALOG_H
#define SPEAKERDIALOG_H

#include <QDialog>

#include "speaker.h"

namespace Ui {
class SpeakerDialog;
}

class SpeakerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpeakerDialog(QWidget *parent = 0);
    SpeakerDialog(const Speaker& edit, QWidget *parent = 0);
    ~SpeakerDialog();

signals:
    void speakerInserted(Speaker spk);
    void speakerCancelled();

private slots:
    void onVendorTextChanged(QString text);
    void onModelTextChanged(QString text);
    void onSpeakerAccepted();
    void onSpeakerRejected();

private:
    Ui::SpeakerDialog *ui;
    QString oldVendor;
    QString oldModel;
};

#endif // SPEAKERDIALOG_H
