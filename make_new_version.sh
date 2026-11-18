#!/bin/sh
#
PROJECT=qspeakers

if [ "$1" = "" ]
then
	echo need version string
	exit 1
fi

echo increment version number in .pro and metainfo.xml files ?
read YES
if [ "$YES" = "y" ]
then
	echo modifying $PROJECT.pro
	sed -i "s/^\(VERSION *= *\).*$/\1$1/" $PROJECT.pro
	NOW=`date +%Y-%m-%d`
	echo modifying *.$PROJECT.metainfo.xml
	sed -i "/<release version=\"$1\"/d" *.$PROJECT.metainfo.xml
	sed -i "s/^\( *<releases>\)$/\1\n    <release version=\"$1\" date=\"$NOW\" type=\"stable\" \/>/" *.$PROJECT.metainfo.xml
else
	exit 0
fi

echo add and commit modifications ?
read YES
if [ "$YES" = "y" ]
then
	echo git add $PROJECT.pro
	git add $PROJECT.pro *.$PROJECT.metainfo.xml
	echo git commit -m "'Increment version number to $1'"
	git commit -m "'Increment version number to $1'"
else
	exit 0
fi

echo push commit and tag for "$1" ?
read YES
if [ "$YES" = "y" ]
then
	echo "tagging as $1 and pushing on github ghmaster:master"
	git push github ghmaster:master && git tag "$1" && git push github "$1"
else
	exit 0
fi
