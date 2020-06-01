#ifndef SYSTEM_H
#define SYSTEM_H

#include "speaker.h"
#include "box.h"
#include "sealedbox.h"
#include "portedbox.h"
#include "bandpassbox.h"

class System
{
public:
    System(const Speaker &s, const SealedBox *b, unsigned int number = 1);
    System(const Speaker &s, const PortedBox *b, unsigned int number = 1);
    System(const Speaker &s, const BandPassBox *b, unsigned int number = 1);
    double response(double f);
    void render(QPainter *painter, const QRectF& area);

private:
    Speaker speaker;
    const Box* box;
    int type;
    int sibling; /* number of same drivers (not for push-pull!) */
};

#endif // SYSTEM_H
