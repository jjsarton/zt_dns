# ZT_DNS

This project is constitued from 3 components which shall help to call by
name the hosts within a zerotier network. Remembering name is easier as an
IP Address.

These components are intended to work on UNIX systems as Linux.  
  
Tte Domain Name Resolution (DNS) relie on an server which return the IP Addresse for a given host or it name according to the query (Name, IP
Addresse).

The Client (your computer, smartphone, ...) use different methode in order to resolve the IP addresse or name.

Classical UNIX System use libraries spezialized for different publishing methode. These libraries are consulted according to the order within a configurations file (**/etc/nsswitch.conf**).

Windows System is able to send DNS queries to the Server configured on an
particular interface (For our case the zerotier network interface). Of course you must yourself configure the Address of your zerotier DNS Server.

Other Systems don't allow one of the allready mentioned methode.

In order to solve the problems **zt_dns** is to be installed on a Linux
system as for example a Raspberry Pi.

**Zt_dns** will ask priodically my.zerotier.com, store the name and IP for each zerotier member and provide a zone file for the **unbound** DNS server.

The **unbound** DNS server shall forward the queries, for hosts or server external to the zerotier network, to a further resolver located for example on your DSL router. This feature will allow to provide the DNS informations for the zerotier and your LAN network as well as for systems located within the internet.

You may be able to bind all your system as NAS or Smartphone in all networks and reach the wanted systems by name.

You may also use the nss components for UNIX like systems providing nsswitch. This offer some advantage for nomadic systems.

The nsswitch feature require to configure **zt\_dns** accordingly. On your System (Linux) you will have to install the library **libnss\_zt.so.2** and the little server **zt\_nss**.

**zt\_nss** get query from the library **libnss\_zt.so.2**, send then to **zt\_dns** and the results back to the library.

## Installation

Edit first the files **etc/zt\_dns.conf** or **etc/zt\_nss.conf** according to your needs.

### DNS server

For the DNS Server install the required development libraries (see file
**zt\_dns.md** and issue as root:

```
make install-dns
```

Don't forget to configure your DHCP server so that the DNS-address send to the client is the address for your zt-DNS server.

### NSS helper

For the client part **zt\_dns** issue, as root, the command:

```
make install-nss
```

Modifiy the file **/etc/resolv.conf** (see **zt\_nss.md**).

See also the file **zt\_nss.md**

## Uninstall

launch:

```
make uninstall
```

