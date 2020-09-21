#!/bin/sh
#
# Microsoft® Windows® 64bits & 32bit cross builder script
#

if [ "$1" = "" ]
then
	ARCH="x86_64"
else
	ARCH="$1" # i686 or x86_64
fi

PATH=/usr/lib/mxe/usr/bin:$PATH
cd ..
/usr/bin/make distclean
/usr/lib/mxe/usr/bin/${ARCH}-w64-mingw32.static-qmake-qt5 -config release
/usr/bin/make
cd nsis
/usr/bin/makensis QSpeakers.nsi
