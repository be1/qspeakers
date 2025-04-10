#ifndef BOX_H
#define BOX_H

#include <QDomElement>
#include <QDomDocument>
#include <QRectF>
#include <QPainter>

#define BOX_SEALED 0
#define BOX_PORTED 1
#define BOX_BANDPASS 2

#define BOX_RENDER_MARGINS 13

class Box
{
public:

    QDomElement toDomElement(QDomDocument& doc) const;
    void fromDomElement(const QDomElement& e);
    virtual void render(QPainter *painter, const QRectF& area) const = 0;
};

#endif // BOX_H
