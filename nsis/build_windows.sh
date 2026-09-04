#!/bin/sh
#

if ! [ -f /usr/bin/makensis ]; then
	echo "makensis must be installed"
	exit 1
fi
if ! [ -d $HOME/src/mxe/usr/bin ]; then
	echo "mxe binaries must be installed under $HOME/src/mxe"
	exit 1
fi

if [ "$1" = "" ]
then
	ARCH="x86_64"
else
	ARCH="$1" # i686 or x86_64
fi

PATH=$HOME/src/mxe/usr/bin:$PATH
make distclean
${ARCH}-w64-mingw32.static-qt6-qmake ..
make
makensis QSpeakers.nsi
