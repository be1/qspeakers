#include <math.h>

#include "optimizer.h"

Optimizer::Optimizer(const Speaker& speaker, SealedBox *box, int sibling) :
    speaker(speaker),
    box(box),
    type(BOX_SEALED),
    sibling(sibling)
{
}

Optimizer::Optimizer(const Speaker &speaker, PortedBox *box, int sibling) :
    speaker(speaker),
    box(box),
    type(BOX_PORTED),
    sibling(sibling)
{
}

Optimizer::Optimizer(const Speaker &speaker, BandPassBox *box, int sibling) :
    speaker(speaker),
    box(box),
    type(BOX_BANDPASS),
    sibling(sibling)
{
}

void Optimizer::genericOptimizeBox()
{
    if (type == BOX_SEALED) {
        SealedBox *b = (SealedBox *)box;
        double qr = 0.707 / speaker.getQts();
        double vr = pow(qr, 2.0) - 1;
        b->setVolume(speaker.getVas() * sibling / vr);
    } else if (type == BOX_PORTED) {
        PortedBox *b = (PortedBox *)box;
        /* approx. "natural max flat", a good starting point */
        b->setBoxVolume(20 * speaker.getVas() * sibling * pow(speaker.getQts(), 3.3));
        b->setResFreq(speaker.getFs() * pow(speaker.getVas() * sibling / b->getBoxVolume(), 0.31));
        b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
    } else {
        /* default optimization for 0dB ripple (s) and 0dB gain (pa)*/
        double s = 0.7;
        double pa = 0.0;
        bandpassAlignS_Pa(s, pa);
    }
}

void Optimizer::portedAlignBessel()
{
    PortedBox *b = (PortedBox *)box;
    b->setBoxVolume(8.0707 * pow(speaker.getQts(), 2.5848) * speaker.getVas() * sibling);
    b->setResFreq(0.3552 * pow(speaker.getQts(), -0.9649) * speaker.getFs());
    b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
}

void Optimizer::portedAlignBullock()
{
    PortedBox *b = (PortedBox *)box;
    double qts = speaker.getQts();
    b->setBoxVolume(qts * speaker.getVas() * sibling * (4.96 * qts - 0.136));
    b->setResFreq(speaker.getFs());
    b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
}

void Optimizer::portedAlignKeele_Hoge()
{
    PortedBox *b = (PortedBox *)box;
    b->setBoxVolume(speaker.getVas() * sibling * 5.2358 * pow(speaker.getQts(), 2.1687));
    b->setResFreq(speaker.getFs());
    b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
}

void Optimizer::portedAlignLegendre()
{
    PortedBox *b = (PortedBox *)box;
    b->setBoxVolume(10.728 * pow(speaker.getQts(), 2.4186) * speaker.getVas() * sibling);
    b->setResFreq(0.3802 * pow(speaker.getQts(), -1.0657) * speaker.getFs());
    b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
}

void Optimizer::portedAlignModerate_Inf()
{
    PortedBox *b = (PortedBox *)box;
    /* use M4 Moderate alignment (see http://www.mzbinden.ch/ventedalignments/index.html) */
    b->setBoxVolume((2.52 * speaker.getQts() - 0.35) * speaker.getVas() * sibling);
    b->setResFreq(0.32 * sqrt((1.0/pow(speaker.getQts(), 2.0)) + 3.38) * speaker.getFs());
    b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
}

void Optimizer::bandpassAlignS_Pa(double s, double pa)
{
    BandPassBox *b = (BandPassBox *)box;
    /* see http://www.diysubwoofers.org/bnd/4thord1.htm */
    double qbp = pow((pow(10.0,(-pa/40.0)) * 2 * s ), -1);
    double fb  = qbp * speaker.getFs() / speaker.getQts();
    double vf  = pow(2 * s * speaker.getQts(), 2) * speaker.getVas() * sibling;
    double vr  = speaker.getVas() * sibling / (pow(qbp / speaker.getQts(), 2) - 1);
    b->setSealedBoxVolume(vr);
    b->setPortedBoxVolume(vf);
    b->setPortedBoxResFreq(fb);
    b->updatePortedBoxPorts(speaker.getSd() * sibling, speaker.getXmax());
}
