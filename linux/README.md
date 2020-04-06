First time:

1) petalinux-create -t project -n iceberg-wib --template zynqMP
2) Make changes to default device tree if needed (ethernet/sdcard)

Updating hardware description (device tree):

1) copy DUNE_WIB.hdf from vivado
2) petalinux-config --get-hw-description=./

Building linux distro:

petalinux-build

Building boot image:

1) copy DUNE_WIB.bit from vivaod (file, export, export bitstream)
2) petalinux-package --boot --fsbl images/linux/zynqmp_fsbl.elf --pmufw images/linux/pmufw.elf --fpga DUNE_WIB.bit --u-boot --force
3) Copy images/linux/BOOT.bin to /boot/ on WIB
