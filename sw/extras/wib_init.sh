#!/bin/sh
echo "Register 0xA00C008C reads $(peek 0xA00C008C)"
python3 /etc/rc5.d/set_WIB_ip.py $(peek 0xA00C008C)
echo "Starting wib_server"
/bin/wib_server 2>/var/log/wib_server.err >/var/log/wib_server.log &

#Evidentally this crashes the whole system somehow...
#echo "Starting xvcserver"
#/usr/bin/xvcserver  2>/var/log/xvcserver.err >/var/log/xvcserver.log &
