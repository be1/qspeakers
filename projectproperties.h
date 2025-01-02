#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <QString>
#include <QDomElement>
#include <QDomDocument>

class ProjectProperties
{
public:
    QDomElement toDomElement(QDomDocument &doc) const;
    void fromDomElement(const QDomElement &el);
    void clear();

    QString title;
    QString note;
};

#endif // PROJECTPROPERTIES_H
