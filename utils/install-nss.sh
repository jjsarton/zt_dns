#!/bin/sh
unset T
#T="echo"

OS=`utils/detect.sh`
if [ $OS != "Linux" ]
then
	echo "Your system don't support zt_nss"
	exit 1
fi
if grep '<' etc/zt_nss.conf > /dev/null
then
	echo Please edit first the file etc/zt_dns.conf
	exit 1
fi
$T cp zt_nss /usr/sbin/

if [ -d /usr/lib64 ]
then
	$T cp libnss_zt.so.2 /usr/lib64/libnss_zt.so.2
else
	$T cp libnss_zt.so.2 /usr/lib/libnss_zt.so.2
fi 
$T cp  etc/zt_nss.service /etc/sysstemd/system
$T cp  etc/zt_nss.conf /etc/
$T systemctl daemon-reload
$T systemctl enable zt_nss
$T systemctl start zt_nss
