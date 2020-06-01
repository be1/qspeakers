#ifndef SEALEDBOX_H
#define SEALEDBOX_H

#include <QDomElement>

#include "box.h"

class SealedBox : public Box
{
public:
    SealedBox(double vol = 0.01);
    void setVolume(double vol);
    double getVolume(void) const;

    QDomElement toDomElement(QDomDocument& doc) const;
    void fromDomElement(const QDomElement& e);
    void render(QPainter *painter, const QRectF& area) const;
private:
    double volume;
};

#endif // SEALEDBOX_H
