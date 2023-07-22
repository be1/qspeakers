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

    // TSPs
    double fs; // Hz (resonance frequency)
    double vas; // L (equivalent volume of air)
    double re; // Ohm (DC resistance)
    double qts; // unitless (total Q)
    double sd; // m² (effective piston area)
    double z; // Ohm (nominal impedance)
    double qms; // unitless (mechanical Q)
    double qes; // unitless (electrical Q)
    double spl; // dB (sensitivity at 1W power input, in 1m distance)

    // Voice coil parameters
    double pe; // W (max power in normal use)
    double le; // mH (voice coil inductance)
    double xmax; // mm (linear excursion)
    double bl; // Tm (force factor)
    int vc; // number of voice coils

    // Mechanical data
    double dia; // m (cutout diameter)
};

Q_DECLARE_METATYPE(Speaker)

#endif // SPEAKER_H
