#!/bin/bash
#mkimage -c none -A arm -T script -d boot.cmd images/linux/boot.scr
petalinux-package --fsbl pre-built/linux/images/zynq_fsbl_plnx2019.elf --boot --u-boot --force --fpga ../*.bit
