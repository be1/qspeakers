#include "mainwindow.h"
#include "importexport.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Herewe");
    a.setOrganizationDomain("herewe");
    a.setApplicationName("QSpeakers");
    a.setApplicationVersion(VERSION " (" REVISION ")");

    QString locale = QLocale::system().name();
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator qspeakersTranslator;
    if (!qspeakersTranslator.load(TARGET "_" + locale, "locale"))
#ifdef __mswin
        qspeakersTranslator.load(TARGET "_" + locale, QCoreApplication::applicationDirPath() + QDir::separator() + "locale");
#else
        qspeakersTranslator.load(TARGET "_" + locale, DATADIR "/" TARGET "/locale");
#endif
    a.installTranslator(&qspeakersTranslator);


    QCommandLineParser parser;
    parser.setApplicationDescription("Loudspeaker enclosure computation program.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("project", QCoreApplication::translate("main", "Project file to work on."));

    parser.process(a);
    const QStringList args = parser.positionalArguments();

    if (args.length() > 0)
        ImportExport::setSavePath(args.at(0));

    MainWindow w; /* must be created _after_ setSavePath */
#ifdef __mswin
    QString iconpath = QCoreApplication::applicationDirPath() + QDir::separator() + TARGET + ".png";
#else
    QString iconpath = QString(DATADIR "/icons/hicolor/scalable/apps/" TARGET ".svg");
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    if (QFileInfo::exists(iconpath))
#else
    if ((QFileInfo(iconpath)).exists())
#endif
        w.setWindowIcon(QIcon(iconpath));
    else
        w.setWindowIcon(QIcon(TARGET ".png"));

    if (argc > 1)
        w.setWindowFilePath(argv[argc - 1]);

    w.show();

    return a.exec();
}
