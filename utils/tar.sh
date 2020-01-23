#!/bin/sh
FILE="zt_dns/*.[ch] zt_dns/*.md zt_dns/Makefile zt_dns/etc zt_dns/utils"
cd ..
tar -cf zt_dns/zt_dns.tar $FILE
