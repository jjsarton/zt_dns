#!/bin/bash
if [ -e /etc/zt_dns.conf ]
then
	systemctl stop unbound
	systemctl stop zt_dns
	systemctl disable unbound
	systemctl disable zt_dns

	if test -d /etc/unbound/unbound.conf-PreZT
	then
		rm /etc/unbound/unbound.conf
		mv /etc/unbound/unbound.conf-PreZT /etc/unbound/unbound.conf
	fi

	rm -fr /etc/unbound/zones

	if test -e /etc/systemd/system/zt_dns.service
	then
		rm /etc/systemd/system/zt_dns.service
		systemd daemon-reload
	fi

	if test -e /etc/zt_dns.conf
	then
		rm /etc/zt_dns.conf
	fi

	if test -e /usr/sbin/zt_dns
	then
		rm /usr/sbin/zt_dns
	fi
fi

if [ -e /etc/zt_nss.conf ]
then
	systemctl disable zt_nss
	systemctl stop zt_nss
	if [ -d /usr/lib64 ]
	then
		rm /usr/lib64/libnss_zt.so.2
	else
		rm /usr/lib/libnss_zt.so.2
	fi
	rm /etc/zt_nss.conf
	rm /etc/systemd/system/zt_nss.services
	systemctl daemon-reload
fi
systemctl daemon-reload
exit 0
