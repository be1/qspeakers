#!/bin/sh
#

if [ "$1" = "" ]
then
	echo need version string
	exit 1
fi

echo increment version number in .pro file ?
read YES
if [ "$YES" = "y" ]
then
	sed -i "s/^\(VERSION *= *\).*$/\1$1/" qspeakers.pro
else
	exit 0
fi

# missing metainfo file modification !
#
#echo commit .pro file ?
#read YES
#if [ "$YES" = "y" ]
#then
#	git add qspeakers.pro
#	git commit -m "'Increment version number to $1'"
#else
#	exit 0
#fi
#
#echo push commit and tag for "$1" ?
#read YES
#if [ "$YES" = "y" ]
#then
#	git push github ghmaster:master && git tag "$1" && git push github "$1"
#else
#	exit 0
#fi
