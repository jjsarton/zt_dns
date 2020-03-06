#CC=clang
CC=gcc
CFLAGS=-O2 -g -Wall

#ENTWARE=$(shell test -f /opt/bin/opkg && echo 1)
#
#ifeq ($(ENTWARE),1)
#CFLAGS += -Iinclude -L/opt/lib -D CONF_DIR='"/opt"'
#else
#CFLAGS += -D CONF_DIR='""'
#endif

OS=$(shell utils/detect.sh)
include mk/$(OS).mk

DNSP_FILE = zt_dnsp.c zt_conf.c
DNS_FILE = zt_dns.c zt_conf.c
NSS_FILE = zt_nss.c zt_conf.c
ZT_FILE  = nss_zt.c

all: zt_dns  zt_nss libnss_zt.so.2

zt_dns: $(DNS_FILE)
	$(CC) -o $@ $(DNS_FILE) $(LDFLAGS) -l curl -l json-c  -lpthread $(CFLAGS)

zt_nss: zt_nss.c zt_conf.c
	$(CC) -o $@ $(NSS_FILE) $(CFLAGS)

libnss_zt.so.2: $(ZT_FILE)
	$(CC) -o $@ $< -Wl,-soname,$@ $(CFLAGS) -shared -fPIC

# Header file dependencies
libnss_zt.so.2:  zt_nss.h
zt_nss:          zt_conf.h zt_nss.h
zt_dns:          zt_conf.h


clean:
	@test ! -e qnss || rm qnss;
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

