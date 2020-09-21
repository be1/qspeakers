#include <QLocale>
#include <QDebug>

#include <math.h>

#include "portedbox.h"

PortedBox::PortedBox(double volume, double resfreq, unsigned int portnum, double portdiam, double portlen) :
    box(volume)
{
    resFreq = resfreq;
    portNum = portnum;
    portDiam = portdiam;
    portLen = portlen;
    slotActivated = false;
}

void PortedBox::setBoxVolume(double vol)
{
    box.setVolume(vol);
}

void PortedBox::setPortNum(unsigned int value)
{
    portNum = value;
}

void PortedBox::setPortLen(double len)
{
    portLen = len;
}

void PortedBox::setPortDiam(double diam)
{
    portDiam = diam;
}

void PortedBox::setSlotWidth(double width)
{
    slotWidth = width;
}

void PortedBox::setResFreq(double value)
{
    resFreq = value;
}

void PortedBox::setSlotPortActivated(bool enable)
{
    slotActivated = enable;
}

double PortedBox::getBoxVolume() const
{
    return box.getVolume();
}

unsigned int PortedBox::getPortNum() const
{
    return portNum;
}

double PortedBox::getPortLen() const
{
    if (!slotActivated)
        return portLen;

    double x = getSlotWidth() / getSlotHeight();
    if (x < 1) {
        x = getSlotHeight() / getSlotWidth();
    }

    return (1.1 - log10(x + 1.1) * 0.35) * portLen; /* estimated from https://sites.google.com/site/francisaudio69/ */
}

double PortedBox::getPortDiam() const
{
    return portDiam;
}

bool PortedBox::getSlotPortActivated() const
{
    return slotActivated;
}

double PortedBox::getSlotWidth() const
{
    return slotWidth;
}

double PortedBox::getSlotHeight() const
{
    /* width is manual, height is computed from eq. diam. */
    return (PI * pow(portDiam, 2.0)) / (4.0 * slotWidth);
}

double PortedBox::getResFreq() const
{
    return resFreq;
}

void PortedBox::updatePorts(double sd, double xmax)
{
    /* compute minimum port diameter and optionaly set it */
    double vol = sd * xmax * 0.001; /* m³ */
    double dmin = 100 * (20.3 * pow((pow(vol, 2.0) / getResFreq()), 0.25)) / sqrt(getPortNum()); /* cm */
    setPortDiam(dmin);
    updateSlots();
    updatePortsLength();
}

void PortedBox::updateSlots()
{
    /* compute square slot */
    double diam = getPortDiam();
    double s = sqrt(PI * pow(diam, 2.0) / 4.0);

    /* consider "width" over "height" and reset if too small */
    if (slotWidth == 0.0)
        slotWidth = s;
}

void PortedBox::updatePortsLength()
{
    /* compute resulting length */
    double diam = getPortDiam();
    double res = getResFreq();
    double vol = getBoxVolume();
    double num = getPortNum();
    double plen = ((23562.5 * pow(diam, 2.0)) * num / (vol * pow(res, 2.0))) - (K * diam);
    setPortLen(plen);
}

QDomElement PortedBox::toDomElement(QDomDocument& doc) const
{
    QLocale c(QLocale::C);
    QDomElement e = Box::toDomElement(doc);

    e.setAttribute("type", "ported");
    e.setAttribute("res", c.toString(resFreq));

    QDomElement b = box.toDomElement(doc);
    e.appendChild(b);

    QDomElement p = doc.createElement("port");

    p.setAttribute("num", c.toString(portNum));
    p.setAttribute("len", c.toString(portLen));
    p.setAttribute("diam", c.toString(portDiam));
    p.setAttribute("width", c.toString(slotWidth));
    p.setAttribute("slot", c.toString(slotActivated ? 1 : 0));

    e.appendChild(p);

    return e;
}

void PortedBox::fromDomElement(const QDomElement &e)
{
    QLocale c(QLocale::C);
    Box::fromDomElement(e);

    if (e.attribute("type") != "ported") {
        qWarning() << __func__ << "wrong box type! (not ported, giving up)";
        return;
    }

    resFreq = c.toDouble(e.attribute("res"));

    QDomElement b = e.elementsByTagName("box").at(0).toElement();
    box.fromDomElement(b);

    QDomElement p = e.elementsByTagName("port").at(0).toElement();

    portNum = c.toUInt(p.attribute("num"));
    portLen = c.toDouble(p.attribute("len"));
    portDiam = c.toDouble(p.attribute("diam"));
    slotWidth = c.toDouble(p.attribute("width"));
    slotActivated = c.toUInt(p.attribute("slot"));
}

void PortedBox::render(QPainter *painter, const QRectF &area) const
{    
    if (!painter)
        return;

#define PORTED_PARAMS 5

    QString params[PORTED_PARAMS];
    qreal tab = area.left();

    painter->drawRoundRect(area.toRect(), 5, 5);

    params[0] = QObject::tr("Vol. %1 L").arg(getBoxVolume());
    if (!slotActivated) {
        params[1] = QObject::tr("Port Diam. %1 cm").arg(getPortDiam());
    } else {
        params[1] = QObject::tr("[Slot %1x%2 cm]").arg(QString::number(getSlotWidth(), 'f', 2), QString::number(getSlotHeight(), 'f', 2));
    }
    params[2] = QObject::tr("Port Len. %1 cm").arg(QString::number(getPortLen(), 'f', 2));
    params[3] = QObject::tr("Port Num. %1").arg(QString::number(getPortNum()));
    params[4] = QObject::tr("Fb %1 Hz").arg(QString::number(getResFreq()));

    for (int i = 0; i < PORTED_PARAMS; i++) {
        QString text = params[i];
        QRectF where(tab, area.top(), area.width() / PORTED_PARAMS, area.height());
        QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
        painter->drawText(where, text, option);
        tab += area.width() / PORTED_PARAMS;
    }
}
