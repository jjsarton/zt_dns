#!/bin/bash
unset T
#T="echo"
OS=`utils/detect.sh`
if [ $OS != "Linux" ]
then
	echo "Uninstallion not provided, do this manually"
	exit 1
fi
if [ -e /etc/zt_dns.conf ]
then
	$T systemctl stop unbound
	$T systemctl stop zt_dns
	$T systemctl disable unbound
	$T systemctl disable zt_dns

	if test -d /etc/unbound/unbound.conf-PreZT
	then
		$T rm /etc/unbound/unbound.conf
		$T mv /etc/unbound/unbound.conf-PreZT /etc/unbound/unbound.conf
	fi

	$T rm -fr /etc/unbound/zones

	if test -e /etc/systemd/system/zt_dns.service
	then
		$T rm /etc/systemd/system/zt_dns.service
		$T systemd daemon-reload
	fi

	if test -e /etc/zt_dns.conf
	then
		$T rm /etc/zt_dns.conf
	fi

	if test -e /usr/sbin/zt_dns
	then
		$T rm /usr/sbin/zt_dns
	fi
fi

if [ -e /etc/zt_nss.conf ]
then
	$T systemctl disable zt_nss
	$T systemctl stop zt_nss
	if [ -d /usr/lib64 ]
	then
		$T rm /usr/lib64/libnss_zt.so.2
	else
		$T rm /usr/lib/libnss_zt.so.2
	fi
	$T rm /etc/zt_nss.conf
	$T rm /etc/systemd/system/zt_nss.services
	$T systemctl daemon-reload
fi
$T systemctl daemon-reload
exit 0
