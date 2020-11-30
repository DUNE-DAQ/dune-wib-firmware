#!/bin/bash

petalinux-package --fsbl pre-built/linux/images/zynq_fsbl_plnx2019.elf --boot --u-boot --force --fpga images/linux/*.bit

cd images/linux/
tar czf bootfs.tar.gz BOOT.BIN boot.scr image.ub 
