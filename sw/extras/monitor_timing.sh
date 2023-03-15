#!/bin/sh

badTimingStatus=false
while true ; do
    if $(/etc/rc5.d/devreg.sh ts_stat | grep -q "0x0\|0x1\|0x7\|0x8"); then
	if $badTimingStatus; then
	    for ((c=1; c<=300;c++ ))
	    do
		/etc/rc5.d/devreg.sh ps_en_in 1
		/etc/rc5.d/devreg.sh ps_en_in 0
	    done
	    badTimingStatus=false
	    echo "Detected recovery of timing status, readjusting I2C clock phase delay" >> /var/log/wib_server.log
	fi
    else
	badTimingStatus=true
    fi
    sleep 2
done
