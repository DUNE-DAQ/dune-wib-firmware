#!/bin/bash

petalinux-package --fsbl ../linux-2019.1/images/linux/zynqmp_fsbl.elf --boot --u-boot --force --fpga images/linux/*.bit

cd images/linux/
tar czf bootfs.tar.gz BOOT.BIN boot.scr image.ub 
