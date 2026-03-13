#!/bin/sh
#

if [ "$1" = "" ]
then
	ARCH="i686"
else
	ARCH="$1" # i686 or x86_64
fi

PATH=$HOME/src/mxe/usr/bin:$PATH
make distclean
${ARCH}-w64-mingw32.static-qt6-qmake ..
make
makensis QSpeakers.nsi
