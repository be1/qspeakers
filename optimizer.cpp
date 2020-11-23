#include <math.h>

#include "optimizer.h"
#include "mainwindow.h"
#include "undocommands.h"

Optimizer::Optimizer(const Speaker& speaker, SealedBox *box, int sibling, QObject* parent) :
    speaker(speaker),
    box(box),
    type(BOX_SEALED),
    sibling(sibling),
    mainwindow(static_cast<MainWindow*>(parent))
{
}

Optimizer::Optimizer(const Speaker &speaker, PortedBox *box, int sibling, QObject *parent) :
    speaker(speaker),
    box(box),
    type(BOX_PORTED),
    sibling(sibling),
    mainwindow(static_cast<MainWindow*>(parent))
{
}

Optimizer::Optimizer(const Speaker &speaker, BandPassBox *box, int sibling, QObject *parent) :
    speaker(speaker),
    box(box),
    type(BOX_BANDPASS),
    sibling(sibling),
    mainwindow(static_cast<MainWindow*>(parent))
{
}

void Optimizer::genericOptimizeBox()
{
    if (type == BOX_SEALED) {
        SealedBox *b = (SealedBox *)box;

        double qr = 0.707 / speaker.getQts();
        double vr = pow(qr, 2.0) - 1;

        if (nullptr == mainwindow) {
            b->setVolume(speaker.getVas() * sibling / vr);
        } else {
            SealedVolumeCommand* com = new SealedVolumeCommand(b->getVolume(), speaker.getVas() * sibling / vr, mainwindow);
            mainwindow->getCommandStack()->beginMacro(QObject::tr("optimizing sealed box"));
            mainwindow->getCommandStack()->push(com);
            mainwindow->getCommandStack()->endMacro();
        }
    } else if (type == BOX_PORTED) {
        /* approx. "natural max flat", a good starting point */
        double vb = (20 * speaker.getVas() * sibling * pow(speaker.getQts(), 3.3));
        double fb = speaker.getFs() * pow(speaker.getVas() * sibling / vb, 0.31);
        portedAlignVb_Fb(vb, fb, QObject::tr("applying 'max flat' alignment"));
    } else {
        /* default optimization for 0dB ripple (s) and 0dB gain (pa)*/
        double s = 0.7;
        double pa = 0.0;
        bandpassAlignS_Pa(s, pa);
    }
}

void Optimizer::portedAlignBessel()
{
    double vb = 8.0707 * pow(speaker.getQts(), 2.5848) * speaker.getVas() * sibling;
    double fb = 0.3552 * pow(speaker.getQts(), -0.9649) * speaker.getFs();
    portedAlignVb_Fb(vb, fb, QObject::tr("applying Bessel alignment"));
}

void Optimizer::portedAlignBullock()
{
    double qts = speaker.getQts();
    double vb = qts * speaker.getVas() * sibling * (4.96 * qts - 0.136);
    double fb = speaker.getFs();
    portedAlignVb_Fb(vb, fb, QObject::tr("applying Bullock alignment"));
}

void Optimizer::portedAlignKeele_Hoge()
{
    double vb = speaker.getVas() * sibling * 5.2358 * pow(speaker.getQts(), 2.1687);
    double fb = speaker.getFs();
    portedAlignVb_Fb(vb, fb, QObject::tr("applying Keele & Hoge alignment"));
}

void Optimizer::portedAlignLegendre()
{
    double vb = 10.728 * pow(speaker.getQts(), 2.4186) * speaker.getVas() * sibling;
    double fb = 0.3802 * pow(speaker.getQts(), -1.0657) * speaker.getFs();
    portedAlignVb_Fb(vb, fb, QObject::tr("applying Legendre alignment"));
}

void Optimizer::portedAlignModerate_Inf()
{
    /* use M4 Moderate alignment (see http://www.mzbinden.ch/ventedalignments/index.html) */
    double vb = (2.52 * speaker.getQts() - 0.35) * speaker.getVas() * sibling;
    double fb = 0.32 * sqrt((1.0/pow(speaker.getQts(), 2.0)) + 3.38) * speaker.getFs();
    portedAlignVb_Fb(vb, fb, QObject::tr("applying Zbinden alignment"));
}

void Optimizer::portedAlignVb_Fb(double vb, double fb, const QString& title)
{
    PortedBox *b = (PortedBox *)box;
    if (nullptr == mainwindow) {
        b->setBoxVolume(vb);
        b->setResFreq(fb);
        b->updatePorts(speaker.getSd() * sibling, speaker.getXmax());
    } else {
        QUndoStack* stack = mainwindow->getCommandStack();
        stack->beginMacro(title);
        PortedVolumeCommand* com1 = new PortedVolumeCommand(b->getBoxVolume(), vb, mainwindow);
        stack->push(com1);
        PortedResFreqCommand* com2 = new PortedResFreqCommand(b->getResFreq(), fb, mainwindow);
        stack->push(com2);
        stack->endMacro();
    }
}

void Optimizer::bandpassAlignS_Pa(double s, double pa)
{
    BandPassBox *b = (BandPassBox *)box;
    /* see http://www.diysubwoofers.org/bnd/4thord1.htm */
    double qbp = pow((pow(10.0,(-pa/40.0)) * 2 * s ), -1);
    double fb  = qbp * speaker.getFs() / speaker.getQts();
    double vf  = pow(2 * s * speaker.getQts(), 2) * speaker.getVas() * sibling;
    double vr  = speaker.getVas() * sibling / (pow(qbp / speaker.getQts(), 2) - 1);
    if (nullptr == mainwindow) {
        b->setSealedBoxVolume(vr);
        b->setPortedBoxVolume(vf);
        b->setPortedBoxResFreq(fb);
        b->updatePortedBoxPorts(speaker.getSd() * sibling, speaker.getXmax());
    } else {
        QUndoStack* stack = mainwindow->getCommandStack();
        stack->beginMacro(QObject::tr("applying bandpass box alignment"));
        BPSealedVolumeCommand* com1 = new BPSealedVolumeCommand(b->getSealedBoxVolume(), vr, mainwindow);
        stack->push(com1);
        BPPortedVolumeCommand* com2 = new BPPortedVolumeCommand(b->getPortedBoxVolume(), vf, mainwindow);
        stack->push(com2);
        BPPortedResFreqCommand* com3 = new BPPortedResFreqCommand(b->getPortedBoxResFreq(), fb, mainwindow);
        stack->push(com3);
        stack->endMacro();
    }
}
