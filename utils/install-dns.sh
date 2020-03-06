#!/bin/bash
unset T
#T="echo"

OS=`utils/detect.sh`
case $OS in
"entware") prefix="/opt";;
"Linux") prefix="";;
"FreeBSD") prefix="/usr/local";;
*) echo "Install manually!"
	exit 1;;
esac
if grep '<' etc/zt_dns.conf > /dev/null
then
	echo Please edit first the file etc/zt_dns.conf
	exit 1
fi

if grep 'correct.me' etc/unbound.conf > /dev/null
then
	echo Please edit first the file etc/unbound.conf
	exit 1
fi

if ! test -e $prefix/etc/unbound/unbound.conf-PreZT
then
	$T mv $prefix/etc/unbound/unbound.conf $prefix/etc/unbound/unbound.conf-PreZT
fi

$T cp zt_dns $prefix/usr/sbin/
$T cp etc/zt_dns.conf $prefix/etc/zt_dns.conf
$T mkdir -p $prefix/etc/unbound/zones
$T cp etc/unbound.conf $prefix/etc/unbound/unbound.conf
if [ $OS == "Linux" ]
then
	$T cp etc/zt_dns.service /etc/systemd/system/zt_dns.service
	$T systemd daemon-reload
	$T systemctl enable unbound
	$T systemctl start unbound
	$T systemctl enable zt_dns
	$T systemctl start zt_dns
else
	echo Refer to documentation for installing the startscripts
fi
exit 0
