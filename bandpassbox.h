#ifndef BANDPASSBOX_H
#define BANDPASSBOX_H

#include <QDomElement>
#include <QDomDocument>
#include <QPainter>
#include <QRectF>
#include "box.h"
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
    void setPortedBoxSlotWidth(double width);
    void setPortedBoxResFreq(double freq);
    void setPortedBoxSlotPortActivated(bool enable);

    void updatePortedBoxPorts(double sd, double xmax);
    void updatePortedBoxPortsLength();
    void updatePortedBoxSlots();

    double getSealedBoxVolume(void) const;
    double getPortedBoxVolume(void) const;
    unsigned int getPortedBoxPortNum(void) const;
    double getPortedBoxPortLen(void) const;
    double getPortedBoxPortDiam(void) const;
    bool getPortedBoxSlotPortActivated(void) const;
    double getPortedBoxSlotWidth() const;
    double getPortedBoxSlotHeight() const;
    double getPortedBoxResFreq(void) const;

    QDomElement toDomElement(QDomDocument& doc) const;   
    void fromDomElement(const QDomElement &e);
    void render(QPainter *painter, const QRectF& area) const;

private:
    SealedBox sealedBox;
    PortedBox portedBox;
};

#endif // BANDPASSBOX_H
