#include <QLocale>
#include <QDebug>

#include "sealedbox.h"

SealedBox::SealedBox(double vol)
{
    volume = vol;
}

void SealedBox::setVolume(double vol)
{
    volume = vol;
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
    if (!painter)
        return;

    painter->drawRoundRect(area.toRect(), 5, 5);

    QString text = QObject::tr("Vol. %1 L").arg(getVolume());
    QRectF where(area.left(), area.top(), area.width(), area.height());
    QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
    painter->drawText(where, text, option);
}
