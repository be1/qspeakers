#ifndef SPEAKERDB_H
#define SPEAKERDB_H

#include <QDomDocument>
#include <QDateTime>
#include <QString>
#include <QList>
#include <QFile>

#include "speaker.h"
#include "config.h"

class SpeakerDb
{
public:
    static bool exists(void);
    static void insertOrReplace(const QString& vendor, const QString& name, const Speaker& speaker);
    static void removeByVendorAndModel(const QString& vendor, const QString& model);
    static Speaker getByVendorAndModel(const QString& vendor, const QString& model);

    static QList<QString> getVendors(void);
    static QList<QString> getModelsByVendor(const QString& vendor);

    static QList<Speaker> getByVendor(const QString& vendor);
    static QList<Speaker> getByFs(double min, double max);
    static QList<Speaker> getByDia(double min, double max);
    static QList<Speaker> getByZ(double min, double max);
    static QList<Speaker> getBySd(double min, double max);
    static QList<Speaker> getByVas(double min, double max);
    static QList<Speaker> getByQts(double min, double max);
    static QList<Speaker> getByQes(double min, double max);
    static QList<Speaker> getByQms(double min, double max);
    static QList<Speaker> getByRe(double min, double max);
    static QList<Speaker> getByXmax(double min, double max);
    static QList<Speaker> getBySpl(double min, double max);
    static QList<Speaker> getByPe(double min, double max);
    static QList<Speaker> getByBL(double min, double max);
    static QList<Speaker> getByVc(int min, int max);

    static QList<Speaker> getByValue(QString var, double min, double max);

    static bool merge(QFile& with);
    static QDateTime lastModified(void);
    static QDateTime pkgInstalled(void);
    static QString pkgPath(void);

private:
    static QString getPath(void);
    static QDomDocument getDoc(void);
    static QString getPkgDbPath(void);

#define DB_FILENAME "qspeakers_db.xml"
#define PKG_DB DATADIR "/qspeakers/" DB_FILENAME
};

#endif // SPEAKERDB_H
