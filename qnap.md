# Install zt\_dns on QNAP NAS

## Prerequisite

You must first install entware. Please refer to https://github.com/Entware/Entware/wiki/Install-on-QNAP-NAS and configure your NAS accordingly.


## Installation
  
Install libssl, libcurl and if you will serve DNS unbound.

```
opkg install libssl
opkg install libcurl
opkk install unbound
```

In Order to compile zt\_dns I had to copy the include files for curl and json-c from my Linux system to the zt\_dns directory. The file are now provided.

Some directories may not are present under /opt. In order to get zt\_dns -c configurationfile -f I had to create a link:

```
cd /opt/lib
ln -s libjson-c.so.4 libjson-c.so
ln -s libssl.so.1.1 libssl.so.1.1
```


## Compiling

On a QNAP you may run unbound, this mean that you shall modify the file etc/unbound.conf as follow:

```
server:
directory: "/opt/etc/unbound"
chroot: "/opt/etc/unbound"
username:"admin"
...
	forward-addr: <IP for your Router>
interface: 0.0.0.0
#interface: ::
```

On my QNAP the local interface has no IPv6 Addresse, so I have disabled the
line interface: ::

In order to compile zt\_dns you must set some environment variable correctly. A shell file is provided by entware, so you can issue

```
source /opt/bin/gcc_env.sh
make install-dns
```

## Installation
 
You should also provide a start file within /opt/etc/init.d, for example on my QNAP NAS **S50zt\_dns**:

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

Refer also to the main document.
