#include <QDebug>
#include <QObject>

#include <cmath>

#include "system.h"

#define Q 7 /* common ported box resonance quality loss */
#define QL 10000 /* common box losses */

System::System(const Speaker& s, const SealedBox *b, unsigned int number) :
    speaker(s),
    box(b),
    type(BOX_SEALED),
    sibling(number)
{
}

System::System(const Speaker& s, const PortedBox *b, unsigned int number) :
    speaker(s),
    box(b),
    type(BOX_PORTED),
    sibling(number)
{
}

System::System(const Speaker& s, const BandPassBox *b, unsigned int number) :
    speaker(s),
    box(b),
    type(BOX_BANDPASS),
    sibling(number)
{
}

double System::response(double f)
{
    if (type == BOX_SEALED) {
        SealedBox *b = (SealedBox *)box;
        double vr = speaker.getVas() * sibling / b->getVolume();
        double qr = sqrt(vr + 1.0);
        double qtc = qr * speaker.getQts();
        double fb = qr * speaker.getFs();
        double fr = pow(f / fb, 2.0);
        double db = 10 * log10(pow(fr, 2.0) / (pow(fr - 1, 2.0) + fr/pow(qtc, 2.0)));
        return db;
    } else if (type == BOX_PORTED) {
        PortedBox *b = (PortedBox *)box;
        double A = pow(b->getResFreq() / speaker.getFs(), 2.0);
        double B = A / speaker.getQts() + b->getResFreq() / (Q * speaker.getFs() * speaker.getQts());
        double C = 1.0 + A + (speaker.getVas() * sibling / b->getBoxVolume()) + b->getResFreq() /
                (Q * speaker.getFs() * speaker.getQts());
        double D = 1.0 / speaker.getQts() + b->getResFreq() / (Q * speaker.getFs());
        double fn2 = pow(f / speaker.getFs(), 2.0);
        double fn4 = pow(fn2, 2.0);
        double db = 10 * log10(pow(fn4, 2.0) / (pow(fn4 - C * fn2 + A, 2.0) + fn2 * pow(D * fn2 - B, 2.0)));
        return db;
    } else {
        BandPassBox *b = (BandPassBox *)box;
        double A = pow(1.0 / (b->getPortedBoxResFreq()), 2.0) * pow(f, 4.0);
        double B = ((1 / QL + (speaker.getFs() / b->getPortedBoxResFreq()) / speaker.getQts()) / b->getPortedBoxResFreq()) * pow(f, 3.0);
        double C = (((1 + speaker.getVas() * sibling / b->getSealedBoxVolume() + speaker.getVas() * sibling / b->getPortedBoxVolume()) *
                     speaker.getFs() / b->getPortedBoxResFreq() + (1 / speaker.getQts()) / QL) * speaker.getFs() / b->getPortedBoxResFreq() + 1) * pow(f, 2.0);
        double D = ((1 / speaker.getQts() + (speaker.getFs() / b->getPortedBoxResFreq()) / QL * (speaker.getVas() * sibling / b->getSealedBoxVolume() + 1)) *
                    speaker.getFs()) * f;
        double E = (speaker.getVas() * sibling / b->getSealedBoxVolume() + 1) * pow(speaker.getFs(), 2);
        double G = A - C + E;
        double H = -B + D;
        double db = 20 * log10(pow(f, 2.0) / sqrt(pow(G, 2.0) + pow(H, 2.0)));
        return db;
    }
}

void System::render(QPainter *painter, const QRectF& area)
{
    QFont orig = painter->font();

    qreal textHeight = painter->fontMetrics().height();
    QTextOption option(Qt::AlignLeft);
    QRectF elementArea(area);
    const int elements = 2;

    QString text = QObject::tr("Driver(s) number: ") + QString::number(sibling);

    QFont font;
    font.setBold(false);
    painter->setFont(font);
    painter->drawText(area, text, option);
    font.setBold(true);
    painter->setFont(font);

    elementArea.moveTop(elementArea.y() + textHeight); /* start after first line */
    elementArea.setHeight((area.height() - textHeight) / elements);
    elementArea.setHeight(elementArea.height() - 2 * textHeight); /* each element has 2 header lines */

    /* elements */
    elementArea.moveTop(elementArea.y() + textHeight); /* blank line */
    painter->drawText(elementArea, QObject::tr("Loudspeaker:"), option);
    elementArea.moveTop(elementArea.y() + textHeight);
    speaker.render(painter, elementArea);

    elementArea.moveTop(elementArea.y() + elementArea.height());

    elementArea.moveTop(elementArea.y() + textHeight); /* blank line */
    painter->drawText(elementArea, QObject::tr("Enclosure:"), option);
    elementArea.moveTop(elementArea.y() + textHeight);
    box->render(painter, elementArea);

    painter->setFont(orig);
}
