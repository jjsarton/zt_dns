# Zt\_nss

**Zt\_nss** act as a relay between **libnss\_zt.so.2** and **zt\_dns**.

This server may be installed on an system which use nsswitch.

## Configuration

The configuration file **/etc/zt_nss.conf** must be adapted according to your needs!

```
nssPort:     9999
nssServerIp: <IP Address for your Zerotier server eg. 10.147.17.1>
ztDomain:    <Your Zerotier domain eg. .domain.zt or empty >
IpDomain:    <Enter the common part for the IPs eg. 10.147.17>

```

**nssPort:** refer to the communication port used by **zt\_dns**. See **zt\_dns.md**.

**nssServerIp:** The zerotier IP Address for the DNS Server must be entered here.

**ztDomain:** If this is present and your domain name is provided **zt\_nss** will check if the query is for this domain. If not the query is not send to the **zt\_dns** server


**IpDomain:** As above but the Subnet IP Address is checked first.

If **ztDomain:** or **IpDomain:** are not present and configured all queries will also be send via the possibly slow connection to the **zt_dns** server.

In Order to integrate **zt\_nss** within the NSS system you must modify the file **/etc/nsswitch.conf**.

This file contain a line beginning with ```hosts:``` eg.

```
hosts:  files mdns4_minimal [NOTFOUND=return] dns myhostname
```

Insert ```zt ``` after files:

```
hosts:  files zt mdns4_minimal [NOTFOUND=return] dns myhostname
```
