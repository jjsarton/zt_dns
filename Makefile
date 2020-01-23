#CC=clang
CFLAGS=-O2 -g -Wall

DNS_FILE = zt_dns.c zt_conf.c
NSS_FILE = zt_nss.c zt_conf.c
ZT_FILE  = nss_zt.c

all: zt_dns  zt_nss libnss_zt.so.2

zt_dns: $(DNS_FILE)
	$(CC) -o $@ $(DNS_FILE) -l curl -l json-c $(CFLAGS)

zt_nss: zt_nss.c zt_conf.c
	$(CC) -o $@ $(NSS_FILE) $(CFLAGS)

libnss_zt.so.2: $(ZT_FILE)
	$(CC) -o $@ $< -Wl,-soname,$@ $(CFLAGS) -shared -fPIC

# Header file dependencies
libnss_zt.so.2:  zt_nss.h
zt_nss:          zt_conf.h zt_nss.h
zt_dns:          zt_conf.h


clean:
	@test ! -e zt_dns || rm zt_dns;
	@test ! -e zt_nss || rm zt_nss;
	@test ! -e libnss_zt.so.2 || rm libnss_zt.so.2;

install-dns: zt_dns
	@utils/install-dns.sh

install-nss: zt_nss libnss_zt.so.2
	@utils/install-nss.sh


uninstall:
	@utils/uninstall.sh

tar:
	@utils/tar.sh

