#!/bin/bash
#mkimage -c none -A arm -T script -d boot.cmd images/linux/boot.scr
petalinux-package --boot --u-boot --force --fpga ../*.bit
