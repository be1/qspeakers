#include "speakerdb.h"

#include <iostream>
#include <algorithm>
#include <QDebug>
#include <QString>
#include <QList>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QStandardPaths>
#endif
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QDomNode>
#include <QDateTime>

QString SpeakerDb::getPath(void)
{
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

    QString path = prefix + QDir::separator() + DB_FILENAME;
    return path;
}

QDomDocument SpeakerDb::getDoc(void)
{
    QDomImplementation impl = QDomDocument().implementation();
    QString name = "QSpeakers";
    QString publicId = "-//HEREWE//DTD QSpeakers 1.0 //EN";
    QString systemId = "http://def.herewe.org/DTD/qspeakers1.dtd";
    QDomDocument doc(impl.createDocumentType(name, publicId, systemId));
    doc.appendChild(doc.createComment("QSpeakers db of Thiele-Small parameters"));
    return doc;
}

QString SpeakerDb::getPkgDbPath()
{
#ifdef __mswin
    return QCoreApplication::applicationDirPath() + QDir::separator() + DB_FILENAME;
#else
    return PKG_DB;
#endif
}

QList<QString> SpeakerDb::getModelsByVendor(const QString& vendor, int *maxchars)
{
    int len = 0;
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList speakers = doc.elementsByTagName("speaker");
    QList<QString> list;

    for (int i = 0; i < speakers.size(); i++) {
        QDomNode s = speakers.at(i);
        QDomElement spk = s.toElement();
        QString vnd = spk.attribute("vendor");
        QString mdl = spk.attribute("model");
        if (vnd == vendor)
            list.append(mdl);

        if (mdl.length() > len)
            len = mdl.length();
    }

    if (maxchars)
        *maxchars = len;

    std::sort(list.begin(), list.end());
    return list;
}

QList<Speaker> SpeakerDb::getByVendor(const QString &vendor)
{
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList xspeakers = doc.elementsByTagName("speaker");
    QList<Speaker> speakers;

    for (int i = 0; i < xspeakers.size(); i++) {
        QDomNode s = xspeakers.at(i);
        QDomElement spk = s.toElement();
        if (spk.attribute("vendor") == vendor) {
            Speaker speaker;
            speaker.fromDomElement(spk);

            speakers.append(speaker);
        }
    }

    return speakers;
}

QList<Speaker> SpeakerDb::getByValue(QString var, double min, double max) {
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList xspeakers = doc.elementsByTagName("speaker");
    QList<Speaker> speakers;

    for (int i = 0; i < xspeakers.size(); i++) {
        QDomNode s = xspeakers.at(i);
        QDomElement spk = s.toElement();
        QLocale c(QLocale::C);
        if (c.toDouble(spk.attribute(var)) >= min && c.toDouble(spk.attribute(var)) <= max) {
            Speaker speaker;
            speaker.fromDomElement(spk);

            speakers.append(speaker);
        }
    }

    return speakers;
}

/* two-way merge, where user db is prioritized (return true if had to resolve conflict) */
bool SpeakerDb::merge(QFile &with)
{
    if (!with.exists())
        return false;

    bool ret = false;
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();
    QDomDocument xml = getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    with.open(QIODevice::ReadOnly);
    xml.setContent(&with);
    with.close();

    QDomNodeList speakerlist = doc.elementsByTagName("speaker");
    QDomNodeList withlist = xml.elementsByTagName("speaker");

    QDomDocument temp = getDoc();
    QDomElement troot = temp.createElement("speakers");
    temp.appendChild(troot);

    /* first way */
    qDebug() << "looking " + with.fileName();
    for (int i = 0; i < withlist.size(); i++) {
        bool conflict = false;
        QDomNode s_ = withlist.at(i);
        QDomElement spk_ = s_.toElement();
        QString vnd_ = spk_.attribute("vendor");
        QString mdl_ = spk_.attribute("model");
        qDebug() << "  element " + with.fileName() + ": " + vnd_ + ":" + mdl_;
        qDebug() << "  looking " + file.fileName();
        for (int j = 0; j < speakerlist.size(); j++) {
            QDomNode s = speakerlist.at(j);
            QDomElement spk = s.toElement();
            QString vnd = spk.attribute("vendor");
            QString mdl = spk.attribute("model");
            //qDebug() << "    element " + file.fileName() + ": " + vnd + ":" + mdl;
            if ((vnd.toUpper() == vnd_.toUpper()) && (mdl.toUpper() == mdl_.toUpper())) {
                qDebug() << "    CONFLICT!";
                conflict = true;
                ret = true;
                /* copy the element from user's db to the temporary xml */
                qDebug() << "    import " + vnd + ":" + mdl;
                QDomElement t = spk.cloneNode(true).toElement();
                t.normalize();
                troot.appendChild(t);
                break;
            }
        }

        if (conflict)
            continue;

        /* no conflict, so append the new speaker */
        qDebug() << "  import " + vnd_ + ":" + mdl_;
        QDomElement t_ = spk_.cloneNode(true).toElement();
        t_.normalize();
        troot.appendChild(t_);
    }


    /* prepend temp in merged */
    QDomDocument merged(temp);
    QDomElement mroot = merged.firstChildElement("speakers");

    QDomNodeList templist = temp.elementsByTagName("speaker");

    /* second way */
    qDebug() << "looking " + file.fileName();
    for (int i = 0; i < speakerlist.size(); i++) {
        bool already = false;
        QDomNode s = speakerlist.at(i);
        QDomElement spk = s.toElement();
        QString vnd = spk.attribute("vendor");
        QString mdl = spk.attribute("model");
        qDebug() << "  element " + file.fileName() + ": " + vnd + ":" + mdl;
        qDebug() << "  looking " + with.fileName();
        for (int j = 0; j < templist.size(); j++) {
            QDomNode s_ = templist.at(j);
            QDomElement spk_ = s_.toElement();
            QString vnd_ = spk_.attribute("vendor");
            QString mdl_ = spk_.attribute("model");
            //qDebug() << "    element " + with.fileName() + ": " + vnd_ + ":" + mdl_;
            if ((vnd.toUpper() == vnd_.toUpper()) && (mdl.toUpper() == mdl_.toUpper())) {
                already = true;
                qDebug() << "    ALREADY!";
                break;
            }
        }

        if (already)
            continue;

        /* not already inserted, so append the old speaker */
        qDebug() << "  import " + vnd + ":" + mdl;
        QDomElement m = spk.cloneNode(true).toElement();
        m.normalize();
        mroot.appendChild(m);
    }

    QDomNodeList mergedlist = merged.elementsByTagName("speaker");

    /* write back */
    QFile wfile(path);
    wfile.open(QIODevice::WriteOnly);
    wfile.write(merged.toByteArray());
    wfile.close();

    return ret;
}

QDateTime SpeakerDb::lastModified()
{
    QFileInfo info(getPath());
    qDebug() << info.filePath() + ":" + info.lastModified().toString();
    return info.lastModified();
}

QDateTime SpeakerDb::pkgInstalled()
{
    QFile pkg_db(getPkgDbPath());
    if (pkg_db.exists()) {
        QFileInfo info(PKG_DB);
        qDebug() << info.filePath() + ": " + info.lastModified().toString();
        return info.lastModified();
    }

    /* necessary if app not 'prod' installed */
    QFile cwd_db("./" DB_FILENAME);
    if (cwd_db.exists()) {
        QFileInfo info("./" DB_FILENAME);
        qDebug() << info.filePath() + ": " + info.lastModified().toString();
        return info.lastModified();
    }

    /* necessary if app not 'prod' installed, but in my qspeakers source dir */
    QFile source_db("../qspeakers/" DB_FILENAME);
    if (source_db.exists()) {
        QFileInfo info("../qspeakers/" DB_FILENAME);
        qDebug() << info.filePath() + ":" + info.lastModified().toString();
        return info.lastModified();
    }

    qWarning() << "no pkg db found, returning current datetime as db access!";
    return QDateTime();
}

QString SpeakerDb::pkgPath()
{
    QFile pkg_db(getPkgDbPath());
    if (pkg_db.exists())
        return getPkgDbPath();

    /* necessary if not 'prod' installed */
    QFile cwd_db("./" DB_FILENAME);
    if (cwd_db.exists())
        return "./" DB_FILENAME;

    /* necessary if not 'prod' installed, but in qspeakers source dir */
    QFile source_db("../qspeakers/" DB_FILENAME);
    if (source_db.exists())
        return "../qspeakers/" DB_FILENAME;

    qWarning() << "no pkg db found, return empty path!";
    return QString();
}

QList<Speaker> SpeakerDb::getByFs(double min, double max)
{
    return getByValue("fs", min, max);
}

QList<Speaker> SpeakerDb::getByDia(double min, double max)
{
    return getByValue("dia", min, max);
}

QList<Speaker> SpeakerDb::getByZ(double min, double max)
{
    return getByValue("z", min, max);
}

QList<Speaker> SpeakerDb::getBySd(double min, double max)
{
    return getByValue("sd", min, max);
}

QList<Speaker> SpeakerDb::getByVas(double min, double max)
{
    return getByValue("vas", min, max);
}

QList<Speaker> SpeakerDb::getByQts(double min, double max)
{
    return getByValue("qts", min, max);
}

QList<Speaker> SpeakerDb::getByQes(double min, double max)
{
    return getByValue("qes", min, max);
}

QList<Speaker> SpeakerDb::getByQms(double min, double max)
{
    return getByValue("qms", min, max);
}

QList<Speaker> SpeakerDb::getByRe(double min, double max)
{
    return getByValue("re", min, max);
}

QList<Speaker> SpeakerDb::getByXmax(double min, double max)
{
    return getByValue("xmax", min, max);
}

QList<Speaker> SpeakerDb::getBySpl(double min, double max)
{
    return getByValue("spl", min, max);
}

QList<Speaker> SpeakerDb::getByPe(double min, double max)
{
    return getByValue("pe", min, max);
}

QList<Speaker> SpeakerDb::getByBL(double min, double max)
{
    return getByValue("bl", min, max);
}

QList<Speaker> SpeakerDb::getByVc(int min, int max)
{
    return getByValue("vc", min, max);
}

QList<QString> SpeakerDb::getVendors(int* maxchars)
{
    int len = 0;
    QString path = getPath();
    QFile file(path);
    QDomDocument doc= getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList speakers = doc.elementsByTagName("speaker");
    QList<QString> list;

    for (int i = 0; i < speakers.size(); i++) {
        QDomNode s = speakers.at(i);
        QDomElement spk = s.toElement();
        QString vnd = spk.attribute("vendor");
        if (list.contains(vnd))
            /* avoid duplicates */
            continue;

        list.append(vnd);

        if (vnd.length() > len)
            len = vnd.length();
    }

    if (maxchars)
        *maxchars = len;

    std::sort(list.begin(), list.end());
    return list;
}

bool SpeakerDb::exists()
{
    QString path = getPath();
    QFile file(path);
    return file.exists();
}

void SpeakerDb::insertOrReplace(const QString& vendor, const QString& model, const Speaker& speaker)
{
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();

    QDomDocument xml = getDoc();
    QDomElement wroot = xml.createElement("speakers");
    xml.appendChild(wroot);

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList speakerlist = doc.elementsByTagName("speaker");
    for (int i = 0; i < speakerlist.size(); i++) {
        QDomNode s = speakerlist.at(i);
        QDomElement spk = s.toElement();
        QString vnd = spk.attribute("vendor");
        QString mdl = spk.attribute("model");
        if ((vnd == vendor) && (mdl == model))
            /* 'delete' it for future insert */
            continue;

        /* copy the element to the writeable xml, otherwise we will have a messy loop */
        QDomElement w = spk.cloneNode(true).toElement();
        w.normalize();
        wroot.appendChild(w);
    }

    if (speaker.isValid()) {
        /* append the new speaker */
        QDomElement n = speaker.toDomElement(xml);

        if (speaker.getVendor() != vendor || speaker.getModel() != model)
            qDebug() << "inserted/updated speaker vendor or name changed.";

        n.normalize();
        wroot.appendChild(n);
    }

    /* write back */
    QFile wfile(path);
    wfile.open(QIODevice::WriteOnly);
    wfile.write(xml.toByteArray());
    wfile.close();
}

void SpeakerDb::removeByVendorAndModel(const QString &vendor, const QString &model)
{
    return insertOrReplace(vendor, model, Speaker());
}

Speaker SpeakerDb::getByVendorAndModel(const QString &vendor, const QString &model)
{
    QString path = getPath();
    QFile file(path);
    QDomDocument doc = getDoc();

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        doc.setContent(&file);
        file.close();
    }

    QDomNodeList speakers = doc.elementsByTagName("speaker");
    Speaker speaker;

    for (int i = 0; i < speakers.size(); i++) {
        QDomNode s = speakers.at(i);
        QDomElement spk = s.toElement();
        QString vnd = spk.attribute("vendor");
        QString mdl = spk.attribute("model");
        if ((vnd == vendor) && (mdl == model)) {
            speaker.fromDomElement(spk);
            break;
        }
    }

    return speaker;
}
