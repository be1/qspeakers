#ifndef SPEAKER_H
#define SPEAKER_H

#include <QString>
#include <QMetaType>
#include <QDomNode>
#include <QDomElement>
#include <QDomDocument>
#include <QPainter>
#include <QRectF>

class Speaker
{

public:
    Speaker();
    Speaker(const Speaker& copy);
    ~Speaker();

    Speaker& operator=(const Speaker& copy);
    bool operator!=(const Speaker& r) const;
    bool operator==(const Speaker& r) const;
    bool isValid(void) const;

    void setVendor(const QString& vnd);
    void setModel(const QString& mdl);

    void setFs(double val);
    void setVas(double val);
    void setRe(double val);
    void setQts(double val);
    void setSd(double val);
    void setXmax(double val);
    void setZ(double val);
    void setLe(double val);
    void setQms(double val);
    void setQes(double val);
    void setSpl(double val);
    void setPe(double val);
    void setBL(double val);
    void setDia(double val);
    void setVc(int val);

    QString getVendor() const;
    QString getModel() const;

    double getFs(void) const;
    double getVas(void) const;
    double getRe(void) const;
    double getQts(void) const;
    double getSd(void) const;
    double getXmax() const;
    double getZ() const;
    double getLe() const;
    double getQms() const;
    double getQes() const;
    double getSpl() const;
    double getPe() const;
    double getBL() const;
    double getDia() const;
    int getVc() const;

    QDomElement toDomElement(QDomDocument& doc) const;
    void fromDomElement(const QDomElement& el);
    void render(QPainter *painter, const QRectF& area);

private:
    QString vendor;
    QString model;

    double fs; // Hz
    double vas; // L
    double re; // Ohm
    double qts; // unitless
    double sd; // m² (emissive surface)
    double xmax; // mm
    double z; // Ohm
    double le; // mH
    double qms; // unitless
    double qes; // unitless
    double spl; // dB (sensitivity)
    double pe; // W (maxpower in normal use)
    double bl; // Tm (force factor)
    double dia; // m (diameter)
    int vc; // number of voice coils
};

Q_DECLARE_METATYPE(Speaker)

#endif // SPEAKER_H
