#!/bin/sh
echo "Register 0xA00C008C reads $(peek 0xA00C008C)"
python3 /etc/rc5.d/set_WIB_ip.py $(peek 0xA00C008C)

/etc/rc5.d/devreg.sh i2c_select 0
for ((c=1; c<=300;c++ ))
do
        /etc/rc5.d/devreg.sh ps_en_in 1
        /etc/rc5.d/devreg.sh ps_en_in 0
done
/etc/rc5.d/si5345_config
/etc/rc5.d/devreg.sh fast_cmd_code 1
/etc/rc5.d/devreg.sh link_mask 0xfff0
/etc/rc5.d/devreg.sh edge_to_act_delay 19
/etc/rc5.d/devreg.sh fast_cmd_code 2
/etc/rc5.d/devreg.sh coldata_rx_reset 1
/etc/rc5.d/devreg.sh coldata_rx_reset 0
/etc/rc5.d/devreg.sh felix_rx_reset 1
/etc/rc5.d/devreg.sh felix_rx_reset 0
/etc/rc5.d/devreg.sh rx_timing_sel 0
/etc/rc5.d/devreg.sh ts_srst 1
/etc/rc5.d/devreg.sh ts_srst 0
/etc/rc5.d/devreg.sh cmd_code_edge 0
/etc/rc5.d/devreg.sh cmd_en_edge 1
/etc/rc5.d/devreg.sh cmd_stamp_sync 0x7fea
/etc/rc5.d/devreg.sh cmd_stamp_sync_en 1
/etc/rc5.d/devreg.sh dts_time_delay 0x56
/etc/rc5.d/devreg.sh align_en 1

echo "Starting wib_server"
/bin/wib_server 2>/var/log/wib_server.err >/var/log/wib_server.log &

#Evidentally this crashes the whole system somehow...
#echo "Starting xvcserver"
#/usr/bin/xvcserver  2>/var/log/xvcserver.err >/var/log/xvcserver.log &
