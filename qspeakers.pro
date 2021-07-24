#-------------------------------------------------
#
# Project created by QtCreator 2014-09-23T08:12:05
#
#-------------------------------------------------

QT       += core gui xml widgets printsupport charts
CONFIG += c++11
VERSION = 1.6.2
REVISION = $$system(git describe --long --tags 2>/dev/null || echo "stable")
TARGET = qspeakers
TEMPLATE = app

win32-g++:HOST=__mswin
unix:HOST=__unix

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

win32-g++:contains(QMAKE_HOST.arch, x86_64):{
    message("Host is 64bit")
}

unix {
    isEmpty(PREFIX): PREFIX = /usr/local
    isEmpty(BINDIR): BINDIR = $$PREFIX/bin
    isEmpty(DATADIR): DATADIR = $$PREFIX/share
}

win32-g++ {
    isEmpty(PREFIX): PREFIX = "C:/Program Files/QSpeakers"
    isEmpty(BINDIR): BINDIR = $$PREFIX
    isEmpty(DATADIR): DATADIR = $$PREFIX
}

config.input = config.h.in
config.output = config.h
QMAKE_SUBSTITUTES += config

SOURCES += main.cpp \
    mainwindow.cpp \
    speakerdialog.cpp \
    speakerdb.cpp \
    speaker.cpp \
    importexport.cpp \
    box.cpp \
    sealedbox.cpp \
    portedbox.cpp \
    bandpassbox.cpp \
    plot.cpp \
    listdialog.cpp \
    searchdialog.cpp \
    system.cpp \
    optimizer.cpp \
    bandpassdialog.cpp

HEADERS  += mainwindow.h \
    speakerdialog.h \
    speakerdb.h \
    speaker.h \
    importexport.h \
    box.h \
    sealedbox.h \
    portedbox.h \
    bandpassbox.h \
    plot.h \
    listdialog.h \
    searchdialog.h \
    system.h \
    optimizer.h \
    bandpassdialog.h \
    undocommands.h

FORMS    += mainwindow.ui \
    speakerdialog.ui \
    listdialog.ui \
    searchdialog.ui \
    bandpassdialog.ui

isEmpty(QMAKE_LRELEASE):QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease

TRANSLATIONS += \
    qspeakers_en.ts \
    qspeakers_fr.ts

LOCALE_DIR = locale

updateqm.input = TRANSLATIONS
updateqm.output = $$LOCALE_DIR/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm $$LOCALE_DIR/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm


unix {
    MANPAGE = "qspeakers.1"
    manpage.input = MANPAGE
    manpage.output = $${MANPAGE}.gz
    manpage.commands = gzip --to-stdout ${QMAKE_FILE_IN} > ${QMAKE_FILE_OUT}
    manpage.CONFIG += no_link target_predeps
    QMAKE_EXTRA_COMPILERS += manpage
}

translations.files = $${LOCALE_DIR}
database.files = "qspeakers_db.xml"
scad.files = sealedbox_template.scad portedbox_template.scad bandpassbox_template.scad \
	sealedbox_cutting_template.scad portedbox_cutting_template.scad bandpassbox_cutting_template.scad

unix {
    target.path = $$BINDIR
    manual.path = $$DATADIR/man/man1
    manual.files = $${MANPAGE}.gz
    manual.CONFIG = no_check_exist
    database.path = $$DATADIR/qspeakers
    translations.path = $$DATADIR/$${TARGET}
    mime.path = $$DATADIR/mime/packages
    mime.files = application-x-$${TARGET}.xml
    desktop.path = $$DATADIR/applications
    desktop.files = $${TARGET}.desktop
    icon.path = $$DATADIR/icons/hicolor/scalable/apps
    icon.files = qspeakers.svg
	scad.path = $$DATADIR/$${TARGET}
    metainfo.path = $$DATADIR/metainfo
    metainfo.files = fr.free.brouits.qspeakers.metainfo.xml
    INSTALLS += target \
            icon \
            manual \
            database \
            translations \
            mime \
            desktop \
            scad \
            metainfo
}

win32-g++ {
    target.path = $$BINDIR
    database.path = $$DATADIR
    translations.path = $$DATADIR/$${TARGET}
	scad.path = $$DATADIR/$${TARGET}
    RC_ICONS += qspeakers.ico
    INSTALLS += target \
            database \
            translations \
            scad
}
