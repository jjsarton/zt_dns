if grep '<' etc/zt_nss.conf > /dev/null
then
	echo Please edit first the file etc/zt_dns.conf
	exit 1
fi
cp zt_nss /usr/sbin/

if [ -d /usr/lib64 ]
then
	cp libnss_zt.so.2 /usr/lib64/libnss_zt.so.2
else
	cp libnss_zt.so.2 /usr/lib/libnss_zt.so.2
fi
cp  etc/zt_nss.service /etc/sysstemd/system
cp  etc/zt_nss.conf /etc/
systemctl daemon-reload
systemctl enable zt_nss
systemctl start zt_nss
