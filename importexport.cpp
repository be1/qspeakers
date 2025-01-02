#include <QtCore>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QStandardPaths>
#endif
#include <QDesktopServices>
#include <QDir>
#include <QDebug>

#include "importexport.h"

QString ImportExport::savePath;

QString ImportExport::getSavePath(void)
{
    if (!ImportExport::savePath.isNull() && !ImportExport::savePath.isEmpty()) {
        return ImportExport::savePath;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QString prefix = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#elif QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QString prefix = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString prefix = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

    QDir dir(prefix);
    if (!dir.exists())
        dir.mkpath(prefix);

    QString path = prefix + QDir::separator() + SAVE_FILENAME;
    return path;
}

bool ImportExport::saveProject(const ProjectProperties& props, const Speaker &speaker, const SealedBox &sbox, const PortedBox &pbox, const BandPassBox &bpbox, int number, int tab)
{
    QString path = ImportExport::getSavePath();

    qDebug() << "exporting" << path;
    QFile file(path);
    /* do not test if it exists, just override it */
    return exportProject(file, props, speaker, sbox, pbox, bpbox, number, tab);
}

bool ImportExport::restoreProject(ProjectProperties &props, Speaker &speaker, SealedBox &sbox, PortedBox &pbox, BandPassBox &bpbox, int* number, int* tab)
{
    QString path = ImportExport::getSavePath();
    qDebug() << "importing" << path;
    QFile file(path);
    return importProject(props, speaker, sbox, pbox, bpbox, number, tab, file);
}

bool ImportExport::exportProject(QFile &file, const ProjectProperties& props, const Speaker &speaker, const SealedBox &sbox, const PortedBox &pbox, const BandPassBox &bpbox, int number, int tab)
{
    QDomDocument xml("QSpeakersProject");
    QDomElement root = xml.createElement("project");
    xml.appendChild(root);

    QDomElement spk = speaker.toDomElement(xml);
    root.appendChild(spk);

    QDomElement xsbox = sbox.toDomElement(xml);
    root.appendChild(xsbox);

    QDomElement xpbox = pbox.toDomElement(xml);
    root.appendChild(xpbox);

    QDomElement xbpbox = bpbox.toDomElement(xml);
    root.appendChild(xbpbox);

    QDomElement xlayout = xml.createElement("layout");
    xlayout.setAttribute("sibling", number);
    root.appendChild(xlayout);

    QDomElement xstate = xml.createElement("state");
    xstate.setAttribute("tab", tab);
    root.appendChild(xstate);

    QDomElement xprops = props.toDomElement(xml);
    root.appendChild(xprops);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    if (!file.write(xml.toByteArray())) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool ImportExport::importProject(ProjectProperties& props, Speaker &speaker, SealedBox &sbox, PortedBox &pbox, BandPassBox &bpbox, int *number, int *tab, QFile &file)
{
    QDomDocument doc("QSpeakersProject");

    if (!file.exists())
        return false;

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.firstChildElement("project");

    QDomNodeList speakers = root.elementsByTagName("speaker");
    if (!speakers.isEmpty()) {
        /* for now, only one speaker is considered */
        QDomElement spk = speakers.at(0).toElement();

        if (!spk.isNull())
            speaker.fromDomElement(spk);
    }

    /* crawl 1st level boxes */
    QDomElement box = root.firstChildElement("box");
    while (!box.isNull()) {
        if (box.attribute("type") == "sealed")
            sbox.fromDomElement(box);
        else if (box.attribute("type") == "ported")
            pbox.fromDomElement(box);
        else if (box.attribute("type") == "bandpass")
            bpbox.fromDomElement(box);
        else
            qWarning() << __func__ << "unrecognized box type";
        box = box.nextSiblingElement("box");
    }

    if (number != nullptr) {
        QDomElement layout = root.firstChildElement("layout");
        if (layout.isNull())
            *number = 1; /* default */
        else
            *number = layout.attribute("sibling", "1").toInt();
    }

    if (tab != nullptr) {
        QDomElement state = root.firstChildElement("state");
        if (state.isNull())
            *tab = 0;
        else
            *tab = state.attribute("tab", "0").toInt();
    }

    props.clear();
    QDomNodeList xprops = root.elementsByTagName("properties");
    if (!xprops.isEmpty())
        props.fromDomElement(xprops.at(0).toElement());

    return true;
}

void ImportExport::setSavePath(const QString &path)
{
    ImportExport::savePath = path;
}
