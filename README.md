# ZT_DNS

This project is constitued from components which shall help to call by
name the hosts within a zerotier network. Remembering name is easier as an
IP Address.

These components are intended to work on UNIX systems as Linux. Refer also to the document synology.md, qnap.dm and freebsd.md. 
  
The Domain Name Resolution (DNS) relie on an server which return the IP Addresse for a given host or it name according to the query (Name, IP
Addresse).

The Client (your computer, smartphone, ...) may use different methodes in order to resolve the IP addresse or name.

Classical UNIX System use spezialized libraries for different publishing methode. These libraries are consulted according to the order within a configurations file (**/etc/nsswitch.conf**).

Windows System is able to send DNS queries to the Server configured on an
particular interface (For our case the zerotier network interface). Of course you must yourself configure the Address of your zerotier DNS Server.

Other Systems don't support well one of the allready mentioned methode.

In order to solve the problems **zt\_dns** is to be installed on a UNIX
system as for example a Raspberry Pi or a Linux NAS System. From the theoritical point of view zt\_dns may also run on a BSD based NAS but such system, often don't allow to install all required libraries or the Linux compatibility components.

**Zt\_dns** will ask periodically my.zerotier.com, store the name and IP for each zerotier member and provide a zone file for the **unbound** DNS server.

The **unbound** DNS server shall forward the queries, for hosts or server external to the zerotier network, to a further resolver located for example on your DSL router. This feature will allow to provide the DNS informations for the zerotier and your LAN network as well as for systems located within the internet.

You may be able to bind all your system as NAS or Smartphone in all networks and reach the wanted systems by name.

You may also use the nss components for UNIX like systems providing nsswitch and loadable nss libraries (Linux). This offer some advantage for nomadic systems.

The nsswitch feature require to configure **zt\_dns** accordingly. On your System (Linux) you will have to install the library **libnss\_zt.so.2** and the little server **zt\_nss**.

**zt\_nss** get query from the library **libnss\_zt.so.2**, send then to **zt\_dns** and return the results back to the library.

## Installation

Edit first the files **etc/zt\_dns.conf** or **etc/zt\_nss.conf** according to your needs.

### DNS server

For the DNS Server install the required development libraries (see file
**zt\_dns.md** and issue as root (only for Linux but not for a NAS System):

```
make install-dns
```

Don't forget to configure your DHCP server so that the DNS-address send to the client is the address for your zt-DNS server.

### NSS helper

This is only for Linux systems! For FreeBSD the code will be compiled but not work. On Linux based NAS systems you shall install this manually

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

## Test helper qnss

In order to test the communication with the "NSS"-Server you may issue:

```
make qnss
```

Qnss is very simple and allow to send the query on port 9999 to the running zt\_dns server and shall print out the address and name according to the query. Example:

```
$ ./qnss ns.example.zt 10.147.17.1
10.147.17.1 ns.example.zt
$ ./qnss 10.147.17.1 10.147.17.1
10.147.17.1 ns.example.zt
```

