#include <QDebug>

#include "bandpassbox.h"

BandPassBox::BandPassBox(double svol, double pvol, double pfreq, unsigned int pnum, double plen, double pdiam) :
    sealedBox(svol),
    portedBox(pvol, pfreq, pnum, plen, pdiam)
{
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

void BandPassBox::setPortedBoxResFreq(double freq)
{
    portedBox.setResFreq(freq);
}

void BandPassBox::updatePortedBoxPorts(double sd, double xmax)
{
    portedBox.updatePorts(sd, xmax);
}

void BandPassBox::updatePortedBoxPortsLength()
{
    portedBox.updatePortsLength();
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

double BandPassBox::getPortedBoxResFreq(void) const
{
    return portedBox.getResFreq();
}

QDomElement BandPassBox::toDomElement(QDomDocument &doc) const
{
    QDomElement e = Box::toDomElement(doc);
    e.setAttribute("type", "bandpass");

    QDomElement b = sealedBox.toDomElement(doc);
    e.appendChild(b);

    QDomElement c = portedBox.toDomElement(doc);
    e.appendChild(c);

    return e;
}

void BandPassBox::fromDomElement(const QDomElement &e)
{
    Box::fromDomElement(e);
    if (e.attribute("type") != "bandpass") {
        qWarning() << __func__ << "wrong box type! (not bandpass, giving up)";
        return;
    }

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
    if (!painter)
        return;

    QString params[6];
    qreal tab = area.left();

    painter->drawRoundRect(area.toRect(), 5, 5);

    params[0] = QObject::tr("Sealed Vol. %1 L").arg(getSealedBoxVolume());
    params[1] = QObject::tr("Ported Vol. %1 L").arg(getPortedBoxVolume());
    params[2] = QObject::tr("Port Diam. %1 cm").arg(getPortedBoxPortDiam());
    params[3] = QObject::tr("Port Len. %1 cm").arg(QString::number(getPortedBoxPortLen(), 'f', 2));
    params[4] = QObject::tr("Port Num. %1").arg(getPortedBoxPortNum());
    params[5] = QObject::tr("Fb %1 Hz").arg(getPortedBoxResFreq());

    for (int i = 0; i < 6; i++) {
        QString text = params[i];
        QRectF where(tab, area.top(), area.width() / 6, area.height());
        QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
        painter->drawText(where, text, option);
        tab += area.width() / 6;
    }
}
