# PetaLinux 2020.1 project for the WIB

## Notes

This was created nominally following instructions in this guide:
https://www.xilinx.com/support/documentation/sw_manuals/xilinx2020_1/ug1144-petalinux-tools-reference-guide.pdf
Additionally, the auto-config options for the kernel and uboot were enabled.

### U-Boot

Some issues with missing Cadence USB Gadget support in u-boot arose, and u-boot
required some manual configuration `petalinux-config -c u-boot` to build without
linker errors.

### Kernel

The Kernel requires a patch (included) to the `macb` driver for ethernet to 
work. Without this, everything will appear fine, but no link will be negotiated.
