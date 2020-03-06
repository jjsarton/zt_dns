#!/bin/sh
OS=`uname`
if [ $OS == "FreeBSD" ]
then
	echo $OS
	exit 0
fi
if [ $OS == "Linux" ]
then
	if [ -f /opt/bin/opkg ]
	then
		echo entware
		exit 0
	fi
fi
echo $OS
