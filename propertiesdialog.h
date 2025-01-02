#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
class PropertiesDialog;
}

class PropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertiesDialog(QWidget *parent = 0);
    ~PropertiesDialog();

    void setTitle(const QString& title);
    void setNote(const QString& note);

    QString getTitle();
    QString getNote();

private:
    Ui::PropertiesDialog *ui;
};

#endif // PROPERTIESDIALOG_H
