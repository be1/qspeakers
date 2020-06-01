#ifndef BANDPASSDIALOG_H
#define BANDPASSDIALOG_H

#include <QDialog>

namespace Ui {
class BandpassDialog;
}

class BandpassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandpassDialog(QWidget *parent = 0);
    ~BandpassDialog();

signals:
    void optimizeRequested(double s, double pa);
    void optimizeCancelled();

private slots:
    void onAccepted();
    void onRejected();

private:
    Ui::BandpassDialog *ui;
};

#endif // BANDPASSDIALOG_H
