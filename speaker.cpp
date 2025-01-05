#include "speaker.h"
#include <QDebug>
#include <QLocale>
#include <cmath>
#include <cfloat>

Speaker::Speaker() :
    fs(0.0),
    vas(0.0),
    re(0.0),
    qts(0.0),
    sd(0.0),
    z(0.0),
    qms(0.0),
    qes(0.0),
    spl(0.0),
    pe(0.0),
    le(0.0),
    xmax(0.0),
    bl(0.0),
    vc(1),
    dia(0.0)
{
}

Speaker::Speaker(const Speaker &copy)
{
    this->fs = copy.getFs();
    this->vas = copy.getVas();
    this->re = copy.getRe();
    this->qts = copy.getQts();
    this->sd = copy.getSd();
    this->z = copy.getZ();
    this->qms = copy.getQms();
    this->qes = copy.getQes();
    this->spl = copy.getSpl();
    this->pe = copy.getPe();
    this->le = copy.getLe();
    this->xmax = copy.getXmax();
    this->bl = copy.getBL();
    this->vc = copy.getVc();
    this->dia = copy.getDia();

    this->vendor = copy.getVendor();
    this->model = copy.getModel();
}

Speaker::~Speaker()
{
    /* nothing to do, just for QMetatype */
}

Speaker &Speaker::operator=(const Speaker &copy)
{
    this->fs = copy.getFs();
    this->vas = copy.getVas();
    this->re = copy.getRe();
    this->qts = copy.getQts();
    this->sd = copy.getSd();
    this->z = copy.getZ();
    this->qms = copy.getQms();
    this->qes = copy.getQes();
    this->spl = copy.getSpl();
    this->pe = copy.getPe();
    this->le = copy.getLe();
    this->xmax = copy.getXmax();
    this->bl = copy.getBL();
    this->vc = copy.getVc();
    this->dia = copy.getDia();

    this->vendor = copy.getVendor();
    this->model = copy.getModel();

    return *this;
}

/* https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/ */
static bool almostEqualRelative(double a, double b, double maxRelDiff = DBL_EPSILON)
{
    double diff = fabs(a - b);
    a = fabs(a);
    b = fabs(b);

    double largest = (b > a) ? b : a;

    if (diff <= largest * maxRelDiff)
        return true;
    return false;
}

bool Speaker::operator==(const Speaker& r) const
{

    return almostEqualRelative(fs, r.getFs()) &&
            almostEqualRelative(vas, r.getVas()) &&
            almostEqualRelative(re, r.getRe()) &&
            almostEqualRelative(qts, r.getQts()) &&
            almostEqualRelative(sd, r.getSd()) &&
            almostEqualRelative(z, r.getZ()) &&
            almostEqualRelative(qms, r.getQms()) &&
            almostEqualRelative(qes, r.getQes()) &&
            almostEqualRelative(spl, r.getSpl()) &&
            almostEqualRelative(pe, r.getPe()) &&
            almostEqualRelative(le, r.getLe()) &&
            almostEqualRelative(xmax, r.getXmax()) &&
            almostEqualRelative(bl, r.getBL()) &&
            almostEqualRelative(dia, r.getDia()) &&
            vc == r.getVc();
}

bool Speaker::operator!=(const Speaker& r) const
{
    return !(*this == r);
}

bool Speaker::isValid() const
{
    return !vendor.isNull() && !model.isNull();
}

void Speaker::setVendor(const QString &vnd)
{
    vendor = vnd;
}

void Speaker::setModel(const QString &mdl)
{
    model = mdl;
}

void Speaker::setFs(double val)
{
    fs = val;
}

void Speaker::setVas(double val)
{
    vas = val;
}

void Speaker::setRe(double val)
{
    re = val;
}

void Speaker::setQts(double val)
{
    qts = val;
}

void Speaker::setSd(double val)
{
    sd = val;
}

void Speaker::setXmax(double val)
{
    xmax = val;
}

void Speaker::setZ(double val)
{
    z = val;
}

void Speaker::setLe(double val)
{
    le = val;
}

void Speaker::setQms(double val)
{
    qms = val;
}

void Speaker::setQes(double val)
{
    qes = val;
}

void Speaker::setSpl(double val)
{
    spl = val;
}

void Speaker::setPe(double val)
{
    pe = val;
}

void Speaker::setBL(double val)
{
    bl = val;
}

void Speaker::setDia(double val)
{
    dia = val;
}

void Speaker::setVc(int val)
{
    vc = val;
}

void Speaker::computeEmptyParams()
{
    if ((qes && qms) || (qes && qts) || (qms && qts)) {
        if (!qts) {
            qDebug() << "computing qts";
            qts = (qes * qms) / (qes + qms);
        } else if (!qms && qts != qes) {
            qDebug() << "computing qms";
            qms = - (qts * qes) / (qts - qes);
        } else if (!qes && qts != qms) {
            qDebug() << "computing qes";
            qes = - (qts * qms) / (qts - qms);
        }
    }

    if (fs && re && qes && vas && sd) {
        if (!bl) {
            qDebug() << "computing bl";
            const double rho = 1.204; // Kg/m³ @ +20° Celsius
            const double c = 343.5; // m/s @ +20° Celsius

            double mms = 1. / (pow(2. * M_PI * fs, 2) * ((vas / 1000.) / (rho * pow(c, 2.) * pow(sd, 2.))));
            bl = sqrt(2 * M_PI * fs * re * mms / qes);
        }
    }

    //FIXME other computations
}

QString Speaker::getVendor() const
{
    return vendor;
}

QString Speaker::getModel() const
{
    return model;
}

double Speaker::getFs() const
{
    return fs;
}

double Speaker::getVas() const
{
    return vas;
}

double Speaker::getRe() const
{
    return re;
}

double Speaker::getQts() const
{
    return qts;
}

double Speaker::getSd() const
{
    return sd;
}

double Speaker::getXmax() const
{
    return xmax;
}

double Speaker::getZ() const
{
    return z;
}

double Speaker::getLe() const
{
    return le;
}

double Speaker::getQms() const
{
    return qms;
}

double Speaker::getQes() const
{
    return qes;
}

double Speaker::getSpl() const
{
    return spl;
}

double Speaker::getPe() const
{
    return pe;
}

double Speaker::getBL() const
{
    return bl;
}

double Speaker::getDia() const
{
    return dia;
}

int Speaker::getVc() const
{
    return vc;
}

QDomElement Speaker::toDomElement(QDomDocument &doc) const
{
    QDomElement e = doc.createElement("speaker");

    e.setAttribute("vendor", vendor);
    e.setAttribute("model", model);

    QLocale c(QLocale::C);
    e.setAttribute("fs", c.toString(fs));
    e.setAttribute("vas", c.toString(vas));
    e.setAttribute("re", c.toString(re));
    e.setAttribute("qts", c.toString(qts));
    e.setAttribute("sd", c.toString(sd));
    e.setAttribute("xmax", c.toString(xmax));
    e.setAttribute("z", c.toString(z));
    e.setAttribute("le", c.toString(le));
    e.setAttribute("qms", c.toString(qms));
    e.setAttribute("qes", c.toString(qes));
    e.setAttribute("spl", c.toString(spl));
    e.setAttribute("pe", c.toString(pe));
    e.setAttribute("bl", c.toString(bl));
    e.setAttribute("dia", c.toString(dia));
    e.setAttribute("vc", c.toString(vc));

    return e;
}

void Speaker::fromDomElement(const QDomElement &el)
{
    vendor = el.attribute("vendor");
    model = el.attribute("model");

    QLocale c(QLocale::C);
    fs = c.toDouble(el.attribute("fs"));
    vas = c.toDouble(el.attribute("vas"));
    re = c.toDouble(el.attribute("re"));
    qts = c.toDouble(el.attribute("qts"));
    sd = c.toDouble(el.attribute("sd"));
    xmax = c.toDouble(el.attribute("xmax"));
    z = c.toDouble(el.attribute("z"));
    le = c.toDouble(el.attribute("le"));
    qms = c.toDouble(el.attribute("qms"));
    qes = c.toDouble(el.attribute("qes"));
    spl = c.toDouble(el.attribute("spl"));
    pe = c.toDouble(el.attribute("pe"));
    bl = c.toDouble(el.attribute("bl"));
    dia = c.toDouble(el.attribute("dia"));
    vc = c.toInt(el.attribute("vc"));

    /* for older db compatibility */
    vc = vc ? vc : 1;
}

void Speaker::render(QPainter *painter, const QRectF &area)
{
    QFont orig = painter->font();

    painter->drawRoundedRect(area.toRect(), 5, 5);

#define PARAMLEN 9

    QString params[PARAMLEN];

    params[0] = QString::fromUtf8("Spl: %1 dB").arg(getSpl());
    params[1] = QString::fromUtf8("Fs: %1 Hz").arg(getFs());
    params[2] = QString::fromUtf8("Qts: %1").arg(getQts());
    params[3] = QString::fromUtf8("Vas: %1 L").arg(getVas());
    params[4] = QString::fromUtf8("Dia: %1 m").arg(getDia());
    params[5] = QString::fromUtf8("Xmax: %1 mm").arg(getXmax());
    params[6] = QString::fromUtf8("Z: %1 Ohm").arg(getZ());
    params[7] = QString::fromUtf8("Re: %1 Ohm").arg(getRe());
    params[8] = QString::fromUtf8("Vc: %1").arg(getVc());

    const int margins = 13; /* pixels */
    qreal tab = area.left() + margins;
    qreal realwidth = area.width() - 2 * margins;

    QString text = getVendor() + " " + getModel();
    QRectF where(tab, area.top(), realwidth, area.height() / 2);

    QTextOption option(Qt::AlignCenter);
    QFont font;
    font.setBold(true);
    painter->setFont(font);

    painter->drawText(where, text, option);

    for (int i = 0; i < PARAMLEN; i++) {
        where.setRect(tab, area.top() + area.height() / 2., realwidth / PARAMLEN, area.height() / 2.);
        text = params[i];
        option.setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
        QFont font;
        font.setBold(false);
        painter->setFont(font);
        painter->drawText(where, text, option);
        tab += realwidth / PARAMLEN;
    }

    painter->setFont(orig);
}
