#ifndef SCADDIALOG_H
#define SCADDIALOG_H

#include <QDialog>

class QSpinBox;
class QLabel;

class ScadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScadDialog(int num, QWidget *parent = nullptr);

    static QList<qreal> getValues(int num, QWidget *parent, bool *ok = nullptr);

private:
    QList<QSpinBox*> m_fields;
};

#endif // SCADDIALOG_H
