#include <QLocale>
#include <QDebug>

#include <cmath>

#include "portedbox.h"

PortedBox::PortedBox(double volume, double resfreq, unsigned int portnum, double portdiam, double portlen) :
    box(volume)
{
    setResFreq(resfreq);
    portNum = portnum;
    setPortDiam(portdiam);
    setPortLen(portlen);
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
    portLen = double ((int)(len * 100) / 100.0);
}

void PortedBox::setPortDiam(double diam)
{
    portDiam = double ((int)(diam * 100) / 100.0);
}

void PortedBox::setSlotWidth(double width)
{
    slotWidth = double ((int)(width * 100) / 100.0);
}

void PortedBox::setResFreq(double value)
{
    resFreq = double ((int)(value * 100) / 100.0);
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

    /* estimated from https://sites.google.com/site/francisaudio69/ */
    return double ((int)((1.1 - log10(x + 1.1) * 0.35) * portLen * 100) / 100.0);
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
    return double ((int)((M_PI * pow(portDiam, 2.0)) / (4.0 * slotWidth) * 100) / 100.0);
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
    double s = sqrt(M_PI * pow(diam, 2.0) / 4.0);

    /* consider "width" over "height" and reset if too small */
    if (slotWidth == 0.0)
        slotWidth = double ((int)(s * 100) / 100.0);
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
    QFont orig = painter->font();

#define PORTED_PARAMS 5

    QString params[PORTED_PARAMS];
    qreal tab = area.left() + BOX_RENDER_MARGINS;
    qreal realwidth = area.width() - 2 * BOX_RENDER_MARGINS;

    painter->drawRoundedRect(area.toRect(), 5, 5);

    params[0] = QObject::tr("Volume: %1 L").arg(QString::number(getBoxVolume(), 'f', 2));
    if (!slotActivated) {
        params[1] = QObject::tr("Port Diameter: %1 cm").arg(QString::number(getPortDiam(), 'f', 1));
    } else {
        params[1] = QObject::tr("Slot port: %1 x %2 cm").arg(QString::number(getSlotWidth(), 'f', 1), QString::number(getSlotHeight(), 'f', 1));
    }
    params[2] = QObject::tr("Port Length: %1 cm").arg(QString::number(getPortLen(), 'f', 1));
    params[3] = QObject::tr("Port #: %1").arg(QString::number(getPortNum()));
    params[4] = QObject::tr("Fb: %1 Hz").arg(QString::number(getResFreq()));

    QFont font;
    font.setBold(false);
    painter->setFont(font);

    for (int i = 0; i < PORTED_PARAMS; i++) {
        QString text = params[i];
        QTextOption option(Qt::AlignVCenter|Qt::AlignLeft);
        qreal textwidth = painter->boundingRect(QRect(0, 0, realwidth, 0), text, option).width() + BOX_RENDER_MARGINS;
        QRectF where(tab, area.top(), qMax(realwidth / PORTED_PARAMS, textwidth), area.height());
        painter->drawText(where, text, option);
        tab += where.width();
    }

    painter->setFont(orig);
}
