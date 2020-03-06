# Install zt\_dns on Synology NAS

## Prerequisite

You must first install entware. Please refer to https://github.com/Entware/Entware/wiki/Install-on-Synology-NAS and configure your NAS accordingly.

## Installation

On a Synology you may run unbound, The unbound file etc/unbound.conf must be corrected as follow:

```
server:
directory: "/opt/etc/unbound"
chroot: "/opt/etc/unbound"
username:"bind"
...
	forward-addr: <IP for your Router>
```

The user bind is present and is normally used for DNS services.

Refer also to the main document.

Don't forget to allow access to your NAS on port 53.

Install libssl, libcurl and if you will serve DNS unbound.

```
opkg install libopenssl
opkg install libcurl
opkg install libjson-c
opkk install unbound-daemon
```

If you don't have the compile environment (gcc,...) install also:

* gcc
* make install-dns

In Order to compile zt\_dns I had to copy the include files for curl and json-c from my Linux system to the zt\_dns directory. The file are now provided.

The default configuration file for zt\_dns shall be read from /opt/etc instead from /etc, this foreseen if you compile the code on your NAS


## Compiling

In order to compile zt\_dns you must set some environment variable correctly. A shell file is provided by entware, so you can issue

```
source /opt/bin/gcc_env.sh
make zt_dns
```

## Installation

The Makefile will install zt_dns,... into the correct location
  
You should also provide a start file within /opt/etc/init.d, for example on my Synology NAS **S50zt\_dns**:

```
#!/bin/sh

ENABLED=yes
PROCS=zt_dns
ARGS="-c /opt/etc/zt_dns.conf"
PREARGS=""
DESC=$PROCS
PATH=/opt/sbin:/opt/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

. /opt/etc/init.d/rc.func

```
