#!/bin/bash

petalinux-package --fsbl --boot --u-boot --force --fpga images/linux/*.bit

cd images/linux/
tar czf bootfs.tar.gz BOOT.BIN boot.scr image.ub 
