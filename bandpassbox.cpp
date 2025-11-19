#include <QDebug>
#include <QObject>
#include <QLocale>

#include "bandpassbox.h"

BandPassBox::BandPassBox(double svol, double pvol, double pfreq, unsigned int pnum, double plen, double pdiam, double qloss) :
    sealedBox(svol),
    portedBox(pvol, pfreq, pnum, plen, pdiam)
{
    qLossFactor = qloss;
}

void BandPassBox::setLossQualityFactor(double newQualityFactor)
{
    qLossFactor = newQualityFactor;
}

void BandPassBox::setSealedBoxVolume(double vol)
{
    sealedBox.setVolume(vol);
}

void BandPassBox::setPortedBoxVolume(double vol)
{
    portedBox.setBoxVolume(vol);
}

void BandPassBox::setPortedBoxPortNum(unsigned int val)
{
    portedBox.setPortNum(val);
}

void BandPassBox::setPortedBoxPortLen(double len)
{
    portedBox.setPortLen(len);
}

void BandPassBox::setPortedBoxPortDiam(double diam)
{
    portedBox.setPortDiam(diam);
}

void BandPassBox::setPortedBoxSlotWidth(double width)
{
    portedBox.setSlotWidth(width);
}

void BandPassBox::setPortedBoxResFreq(double freq)
{
    portedBox.setResFreq(freq);
}

void BandPassBox::setPortedBoxSlotPortActivated(bool enable)
{
    portedBox.setSlotPortActivated(enable);
}

void BandPassBox::updatePortedBoxPorts(double sd, double xmax)
{
    portedBox.updatePorts(sd, xmax);
}

void BandPassBox::updatePortedBoxPortsLength()
{
    portedBox.updatePortsLength();
}

void BandPassBox::updatePortedBoxSlots()
{
    portedBox.updateSlots();
}

double BandPassBox::getLossQualityFactor() const
{
    return qLossFactor;
}

double BandPassBox::getSealedBoxVolume() const
{
    return sealedBox.getVolume();
}

double BandPassBox::getPortedBoxVolume() const
{
    return portedBox.getBoxVolume();
}

unsigned int BandPassBox::getPortedBoxPortNum() const
{
    return portedBox.getPortNum();
}

double BandPassBox::getPortedBoxPortLen() const
{
    return portedBox.getPortLen();
}

double BandPassBox::getPortedBoxPortDiam() const
{
    return portedBox.getPortDiam();
}

bool BandPassBox::getPortedBoxSlotPortActivated() const
{
    return portedBox.getSlotPortActivated();
}

double BandPassBox::getPortedBoxSlotWidth() const
{
    return portedBox.getSlotWidth();
}

double BandPassBox::getPortedBoxSlotHeight() const
{
    return portedBox.getSlotHeight();
}

double BandPassBox::getPortedBoxResFreq(void) const
{
    return portedBox.getResFreq();
}

QDomElement BandPassBox::toDomElement(QDomDocument &doc) const
{
    QLocale c(QLocale::C);

    QDomElement e = Box::toDomElement(doc);
    e.setAttribute("type", "bandpass");
    e.setAttribute("ql", c.toString(qLossFactor));

    QDomElement s = sealedBox.toDomElement(doc);
    e.appendChild(s);

    QDomElement p = portedBox.toDomElement(doc);
    e.appendChild(p);

    return e;
}

void BandPassBox::fromDomElement(const QDomElement &e)
{
    Box::fromDomElement(e);
    if (e.attribute("type") != "bandpass") {
        qWarning() << __func__ << "wrong box type! (not bandpass, giving up)";
        return;
    }

    QLocale c(QLocale::C);

    bool ok = false;

    qLossFactor = c.toDouble(e.attribute("ql"), &ok);
    if (!ok)
        qLossFactor = 10000;

    QDomElement b = e.firstChildElement("box");
    while (!b.isNull()) {
        if (b.attribute("type") == "sealed")
            sealedBox.fromDomElement(b);
        else if (b.attribute("type") == "ported")
            portedBox.fromDomElement(b);
        b = b.nextSiblingElement("box");
    }
}

void BandPassBox::render(QPainter *painter, const QRectF &area) const
{
    QFont orig = painter->font();

    QString params[6];
    qreal tab = area.left() + BOX_RENDER_MARGINS;
    qreal realwidth = area.width() - 2 * BOX_RENDER_MARGINS;

    painter->drawRoundedRect(area.toRect(), 5, 5);

    params[0] = QObject::tr("Sealed Volume: %1 L").arg(QString::number(getSealedBoxVolume(), 'f', 2));
    params[1] = QObject::tr("Ported Volume: %1 L").arg(QString::number(getPortedBoxVolume(), 'f', 2));

    if (!getPortedBoxSlotPortActivated()) {
        params[2] = QObject::tr("Port Diameter: %1 cm").arg(QString::number(getPortedBoxPortDiam(), 'f', 1));
    } else {
        params[2] = QObject::tr("Slot port: %1 x %2 cm").arg(QString::number(getPortedBoxSlotWidth(), 'f', 1), QString::number(getPortedBoxSlotHeight(), 'f', 1));
    }

    params[3] = QObject::tr("Port Length: %1 cm").arg(QString::number(getPortedBoxPortLen(), 'f', 1));
    params[4] = QObject::tr("Port #: %1").arg(getPortedBoxPortNum());
    params[5] = QObject::tr("Fb: %1 Hz").arg(getPortedBoxResFreq());

    QFont font;
    font.setBold(false);
    painter->setFont(font);

    for (int i = 0; i < 6; i++) {
        QString text = params[i];
        QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
        qreal textwidth = painter->boundingRect(QRect(0, 0, realwidth, 0), text, option).width() + BOX_RENDER_MARGINS;
        QRectF where(tab, area.top(), qMax(realwidth / 6, textwidth), area.height());
        painter->drawText(where, text, option);
        tab += where.width();
    }

    painter->setFont(orig);
}
