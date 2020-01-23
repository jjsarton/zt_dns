#!/bin/bash
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

if ! test -e /etc/unbound/unbound.conf-PreZT
then
	mv /etc/unbound/unbound.conf /etc/unbound/unbound.conf-PreZT
fi

cp zt_dns /usr/sbin/
cp etc/zt_dns.conf /etc/zt_dns.conf
mkdir -p /etc/unbound/zones
cp etc/unbound.conf /etc/unbound/unbound.conf
cp etc/zt_dns.service /etc/systemd/system/zt_dns.service

systemd daemon-reload
systemctl enable unbound
systemctl start unbound
systemctl enable zt_dns
systemctl start zt_dns

exit 0
