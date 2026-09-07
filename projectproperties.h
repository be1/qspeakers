#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QPainter>

class ProjectProperties
{
public:
    QDomElement toDomElement(QDomDocument &doc) const;
    void fromDomElement(const QDomElement &el);
    void clear();

    QString title;
    QString note;
    qreal renderNote(QPainter *painter, const QRectF &area);
};

#endif // PROJECTPROPERTIES_H
