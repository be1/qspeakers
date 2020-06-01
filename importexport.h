#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <QFile>

#include "speaker.h"
#include "sealedbox.h"
#include "portedbox.h"
#include "bandpassbox.h"

class ImportExport
{
public:
    /* save the project state into a unique dotfile */
    static void saveProject(const Speaker& speaker, const SealedBox& sbox, const PortedBox& pbox, const BandPassBox& bpbox, int number = 1, int tab = 0);

    /* restore the saved project from the dotfile */
    static void restoreProject(Speaker& speaker, SealedBox& sbox, PortedBox& pbox, BandPassBox& bpbox, int *number, int *tab);

    /* in: speaker, sbox, pbox, bpbox, number, tab
     * out: file
     */
    static void exportProject(QFile& file, const Speaker& speaker, const SealedBox& sbox, const PortedBox& pbox, const BandPassBox& bpbox, int number = 1, int tab = 0);

    /* in: file
     * out: speaker, sbox, pbox, bpbox
     */
    static void importProject(Speaker& speaker, SealedBox& sbox, PortedBox& pbox, BandPassBox& bpbox, int *number, int *tab, QFile &file);

    static void setSavePath(const QString& path);
    static QString getSavePath(void);

private:
    static QString savePath;
#define SAVE_FILENAME "qspeakers_save.xml"
};

#endif // IMPORTEXPORT_H
