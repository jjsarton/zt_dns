# Zt_dns

The zerotier VPN network don't offer a resolver and a connection must be done by entering the IP for the devices. There are a few DNS-Server implementation but all of them don't fullfill my requirement.

* A reverse resolution is not available. 
<br>
May be nice.

* The domain name is branded within the software.
	This is not okay and I prefer to have not to type to much words (.zt is better as .domain.zt).
	
* The home LAN is not served.
<br>
This feature may be good for systems which don't allow to perform the name resolution for both LAN and Zerotier (smartphone, NAS,...)

* Subdomain mostly not supported
<br>
I wan't this for testing web pages,...

* The IP addresse for all system  shall be returned, regardless of there state (powered on/of).
<br>
This avoid sending to much queries to my.zerotier.com


## Requirements

On a Raspberry Pi with Rasbian you must install **libcurl4-gnutls-dev** and **libjson-c-dev** in order to build the binary **zt\_dns**.
Of course you will had to install **unbound**.

On other systemes the name of the required packages may differ. For example for Fedora you will have to install **libcurl-devel** and **libjson-c-devel**.

In oder to compile **zt\_dns** type just make and for the installation make install. The binary will be installed under **/usr/sbin/zt\_dns**, a configuration file **/etc/zt\_dns.conf** and a systemd service file **/etc/systemd/system/zt_dns.service**  will be created.

The content of the /etc/unbound directory will also be modified (add the directory **zones** and empty file **zones/zt-zone**). The file **unbound.conf** will be replaced by ours.

## Configuration

The configuration file **/etc/zt\_dns.conf** must be filled with the values according to you network

```
token:        <your authorisation token>
net:          <your network id>
refresh:      60
zonefile:     /etc/unbound/zones/zt-zone
wildcards:    <empty, 0 or 1>
domain:       <empty or for example .domain.zt or .zt>
tmpFile:      /run/zt-zone
nssPort:      9999
noDNS:        <if present and set to 1 unbound is disabled>
```

This file contain variables which will be read from zt\_dns. The last tree entries are optionals, however I recommand to set _tmpFile:_ specially if you run zt\_dns on a Raspberry Pi.  

<dl>
<dt>token:</dt>
<dd>Generate an authorisation token on your my.zerotier.com page and put it here.</dd>

<dt>net:</dt>
This must be filled in with you network ID, see zerotier.com.</dd>

<dt>refresh:</dt>
<dd>Number of minutes betwenn refreshing the data from your zerotier network. Default value is 60 minutes (no value assigned or declaration not present).</dd>

<dt>zonefile:</dt> 
<dd>This shall, normally contain /etc/unbound/zones/zt-zone. If you have a test unbound installation with a path for an other working directory you may adapt this.</dd>

<dt>tmpFile:</dt>
<dd>If not stated or no assignement the file "/etc/unbound/zones/zt-zone/" will be used directly. On Systems as a Raspberry Pi it will be good to avoid writing to the sd card. **/run/zt-zone** is located within the RAM and will be mounted on the file stated with **zonefile:**</dd>

<dt>nssPort:</dt>
<dd>allows Unix systems as Linux to get the IP and Name for the zerotier host via a nss library. This is usefull if you don't use systemd-resolved.</dd>

<dt>domains:</dt>
<dd>If nothing is entered for **domain:** the FQDN (Full Qualified Domain Name )is to be set in the corresponding my.zerotier.com page (eg. host1.zt or host1.domain.zt).</dd>

<dt>wildcards:</dt>
<dd>If the value '1' is assigned to **wildcards:** queries for subdomains of the systems will be answered (eg. nslookup subdomain.host1.zt return always the address of host1.zt).</dd>

<dt>noDNS:</dt>
<dd>if the value is 1 we will not use Unbound.</dd>
</dl>

### running zt\_dns from the commanline

You may launch ```zt_dns -c /path/to_file``` if you want to use an other configurations file, this must be performed with enought right (root if you use an tmpFile).

With ```zt_dns -j`` zt\_dns will print out the json file send by the my.zerotier.com


## Unbound configuration

```
server:
    directory: "/etc/unbound"
    chroot: "/etc/unbound"
    username: unbound
    interface: 0.0.0.0
    interface: ::
    
    access-control: 0.0.0.0/0 refuse
    access-control: 10.147.17.0/24 refuse
    access-control: 192.168.0.0/16 allow
    access-control: 10.0.0.0/8 allow
    access-control: 10.0.0.0/24 allow
    access-control: 172.16.0.0/12 allow

    so-reuseport: yes
    do-ip6: yes
    do-udp: yes
    so-reuseport: yes
    do-tcp: no
    include: "/etc/unbound/zones/*"

remote-control:
    control-enable: yes
    control-interface: 127.0.0.1
    control-use-cert: "no"

forward-zone:
	name: "."
	forward-addr: 192.168.0.1@53#correct.me
```

This is the file **/etc/unbound/unbound.conf** I use on my raspberry Pi. The zone file is created by zt\_dns and will be reloaded by unbound if a new version is created.

Some value are to be adapted to the system running unbound. See also synology.md eg. qnap.md

The access-control rules tell from wich address range recursion (contacting a further resolver) is denied (refuse) or allowed (allow). For this example access for public addresses is denied and for all local addresses ist allowed excepted for the zerotier Network (range 10.147.17.0/24).



The name for the zerotier hosts is the full qualified domain name eg. _host1.zt_ This has the advandage, that I can append _.zt_ to the hostname and I will get the VPN addresse. If I query only _host_ the IP-Address for my lan will be discovered (forwarding to my Router).

An other advantage, for me, is that my router DHCP-server pass the address for the unbound-DNS-Server so that I can use hosts in the _.zt_ with my NAS systems and also with a smartphone connected via wifi into my LAN.

If you want to have the hosts to be member for the domain _.example.zt_ you should not name your hosts as host1.zt or host1.domain.zt. You should declare the domain wihtin _/etc/zt\_dns.conf_ (see above).

## Launching zt\_dns

**Zt_dns** \[-c \</path/configuration-file\>\] \[-j\]
 
 
 ```
 -c     use the configuration file < /path/configuration-file >
 ```
 
 ```
 -j     print out the answer returned by the zerotier server and exit
 ```

**Zt\_dns** may be lauchend as normal user if unbound is not used
(option -c).



## Configure your Router / DHCP Server

Your DHCP shall announce as DNS resolver the System running *zt\_dns* and *unbound* For example on an AVM Fritz!box 192.168.178.2 (The IPv4 Address) for your DNS Server instead of 192.168.178.1 (the AVM Fritz!box themselve).

Provide also the IPv6 Address for the Resolver if you use IPv6 within your LAN.

## Work arounds

On some systems the daemon systemd-resolved will be running and not work well. If you encountour problems do the following

Make shure that systemd-resolved ist not enabled and running:

```
systemctl disable systemd-resolved
systemctl stop system-resolved
```

First ensure that the file */etc/resolv.conf* is not a link and is configured whith the addresse for your router and then make it unmutable with:

```
chattr +i
```

