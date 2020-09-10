#!/bin/bash
#mkimage -c none -A arm -T script -d boot.cmd images/linux/boot.scr
petalinux-package --fsbl ../linux-2019.1/images/linux/zynqmp_fsbl.elf --boot --u-boot --force --fpga ../*.bit
