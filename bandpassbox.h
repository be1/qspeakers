#ifndef BANDPASSBOX_H
#define BANDPASSBOX_H

#include <QDomElement>

#include "sealedbox.h"
#include "portedbox.h"

class BandPassBox : public Box
{
public:
    BandPassBox(double svol = 0.1, double pvol = 0.01, double pfreq = 1, unsigned int pnum = 1, double plen = 0, double pdiam = 0);

    void setSealedBoxVolume(double vol);
    void setPortedBoxVolume(double vol);
    void setPortedBoxPortNum(unsigned int val);
    void setPortedBoxPortLen(double len);
    void setPortedBoxPortDiam(double diam);
    void setPortedBoxResFreq(double freq);
    void updatePortedBoxPorts(double sd, double xmax);
    void updatePortedBoxPortsLength();

    double getSealedBoxVolume(void) const;
    double getPortedBoxVolume(void) const;
    unsigned int getPortedBoxPortNum(void) const;
    double getPortedBoxPortLen(void) const;
    double getPortedBoxPortDiam(void) const;
    double getPortedBoxResFreq(void) const;

    QDomElement toDomElement(QDomDocument& doc) const;   
    void fromDomElement(const QDomElement &e);
    void render(QPainter *painter, const QRectF& area) const;

private:
    SealedBox sealedBox;
    PortedBox portedBox;
};

#endif // BANDPASSBOX_H
