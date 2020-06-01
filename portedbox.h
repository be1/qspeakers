#ifndef PORTEDBOX_H
#define PORTEDBOX_H

#include <QDomElement>

#include "sealedbox.h"
#define K 0.732 /* correction factor for a half flanged disposed port */

class PortedBox : public Box
{
public:
    PortedBox(double volume = 0.01, double resfreq = 1, unsigned int portnum = 1, double portdiam = 0, double portlen = 0);
    void setBoxVolume(double vol);
    void setPortNum(unsigned int value);
    void setPortLen(double len);
    void setPortDiam(double diam);
    void setSlotWidth(double width);
    void setResFreq(double value);

    double getBoxVolume(void) const;
    unsigned int getPortNum() const;
    double getPortLen(void) const;
    double getPortDiam(void) const;
    double getSlotWidth() const;
    double getSlotHeight() const;
    double getResFreq() const;

    void updateSlots();
    void updatePorts(double sd, double xmax);
    void updatePortsLength();

    QDomElement toDomElement(QDomDocument& doc) const;
    void fromDomElement(const QDomElement& e);
    void render(QPainter *painter, const QRectF& area) const;

private:
    SealedBox box;
    double resFreq;

    unsigned int portNum;
    double portLen;
    double portDiam;
    double slotWidth;
};

#endif // PORTEDBOX_H
