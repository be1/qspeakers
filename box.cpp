#include <QDebug>

#include "box.h"

QDomElement Box::toDomElement(QDomDocument& doc) const
{
    QDomElement e = doc.createElement("box");
    return e;
}

void Box::fromDomElement(const QDomElement &e)
{
    if (e.tagName() != "box")
        qWarning() << __func__ << "wrong tag name while importing box!";
}
