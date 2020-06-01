#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "speaker.h"
#include "box.h"
#include "sealedbox.h"
#include "portedbox.h"
#include "bandpassbox.h"

class Optimizer
{
public:
    Optimizer(const Speaker &speaker, SealedBox *box, int sibling);
    Optimizer(const Speaker &speaker, PortedBox *box, int sibling);
    Optimizer(const Speaker &speaker, BandPassBox *box, int sibling);

    void genericOptimizeBox();

    void portedAlignModerate_Inf();
    void portedAlignLegendre();
    void portedAlignBessel();
    void portedAlignBullock();
    void portedAlignKeele_Hoge();

    void bandpassAlignS_Pa(double s, double pa);
private:
    Speaker speaker;
    Box *box;
    int type;
    int sibling; /* number of speakers (not push-pull!) */
};

#endif // OPTIMIZER_H
