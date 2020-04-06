#!/bin/bash

petalinux-package --boot --fsbl images/linux/zynqmp_fsbl.elf --fpga ../DUNE_WIB.bit  --pmufw images/linux/pmufw.elf --u-boot images/linux/u-boot.elf --force
cp images/linux/BOOT.BIN ../
cp images/linux/image.ub ../
