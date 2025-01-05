#include <QLocale>
#include <QDebug>

#include "sealedbox.h"

SealedBox::SealedBox(double vol)
{
    setVolume(vol);
}

void SealedBox::setVolume(double vol)
{
    volume = double ((int)(vol * 100) / 100.0);
}

double SealedBox::getVolume() const
{
    return volume;
}

QDomElement SealedBox::toDomElement(QDomDocument& doc) const
{
    QDomElement e = Box::toDomElement(doc);
    e.setAttribute("type", "sealed");

    QLocale c(QLocale::C);
    e.setAttribute("volume", c.toString(volume));

    return e;
}

void SealedBox::fromDomElement(const QDomElement &e)
{
    Box::fromDomElement(e);
    if (e.attribute("type") != "sealed") {
        qWarning() << __func__ << "wrong box type! (not sealed, giving up)";
        return;
    }

    QLocale c(QLocale::C);
    volume = c.toDouble(e.attribute("volume"));
}

void SealedBox::render(QPainter *painter, const QRectF &area) const
{
    QFont orig = painter->font();

    painter->drawRoundedRect(area.toRect(), 5, 5);

    QFont font;
    font.setBold(false);
    painter->setFont(font);

    QString text = QObject::tr("Vol. %1 L").arg(getVolume());
    QRectF where(area.left() + BOX_RENDER_MARGINS, area.top(), area.width() - 2 * BOX_RENDER_MARGINS, area.height());
    QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
    painter->drawText(where, text, option);

    painter->setFont(orig);
}
