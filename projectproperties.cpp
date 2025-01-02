#include <QDomElement>
#include <QDomDocument>

#include "projectproperties.h"

QDomElement ProjectProperties::toDomElement(QDomDocument &doc) const
{
    QDomElement props = doc.createElement("properties");

    QDomElement t = doc.createElement("title");
    QDomText tval = doc.createTextNode(this->title);
    t.appendChild(tval);

    QDomElement n = doc.createElement("note");
    QDomText nval = doc.createTextNode(this->note);
    n.appendChild(nval);

    props.appendChild(t);
    props.appendChild(n);

    return props;
}

void ProjectProperties::fromDomElement(const QDomElement &el)
{
    QDomNodeList titles = el.elementsByTagName("title");
    if (!titles.isEmpty()) {
        QDomElement t = titles.at(0).toElement();
        title = t.text();
    }

    QDomNodeList notes = el.elementsByTagName("note");
    if (!notes.isEmpty()) {
        QDomElement n = notes.at(0).toElement();
        note = n.text();
    }
}

void ProjectProperties::clear() {
    title = "";
    note = "";
}
