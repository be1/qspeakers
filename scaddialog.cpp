#include "scaddialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>

ScadDialog::ScadDialog(int num, QWidget *parent) : QDialog(parent)
{
    QFormLayout *lytMain = new QFormLayout(this);

    for (int i = 0; i < num; ++i)
    {
        QLabel *label = nullptr;
        QSpinBox *spinBox = new QSpinBox(this);
	spinBox->setMinimum(1);
	spinBox->setMaximum(1000);
        switch (i) {
            case 0:
                label = new QLabel(tr("Loudspeaker margin (mm)"), this);
		spinBox->setValue(50);
                break;
            case 1:
                label = new QLabel(tr("Wood thick (mm)"), this);
		spinBox->setValue(20);
                break;
            case 2:
                label = new QLabel(tr("Saw thick (mm)"), this);
		spinBox->setValue(1);
                break;
            default:
                break;
        }

        lytMain->addRow(label, spinBox);

        m_fields << spinBox;
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    lytMain->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &ScadDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &ScadDialog::reject);
    Q_ASSERT(conn);

    setLayout(lytMain);
}

QList<qreal> ScadDialog::getValues(int num, QWidget *parent, bool *ok)
{
    ScadDialog *dialog = new ScadDialog(num, parent);

    QList<qreal> list;

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;

    if (ret) {
        foreach (auto field, dialog->m_fields) {
            list << field->value();
        }
    }

    dialog->deleteLater();

    return list;
}
