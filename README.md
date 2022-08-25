# DUNE WIB Firmware

Central repository for development of DUNE WIB firmware and software.

- [DUNE WIB Firmware](#dune-wib-firmware)
  * [Read First](#read-first)
  * [Development](#development)
    + [Getting Started](#getting-started)
    + [Organization](#organization)
  * [Building from Scratch](#building-from-scratch)
    + [Generate bitstream](#generate-bitstream)
      - [Quickly updating the bitstream](#quickly-updating-the-bitstream)
    + [Export hardware definition](#export-hardware-definition)
    + [Create PetaLinux system](#create-petalinux-system)
    + [Creating/Updating boot media](#creating-updating-boot-media)
      - [Alternative method: filesystem tars](#alternative-method--filesystem-tars)
      - [Alternative method: `wib_client` update](#alternative-method---wib-client--update)
  * [Test drive the linux system with QEMU](#test-drive-the-linux-system-with-qemu)
    + [Initial setup](#initial-setup)
    + [Booting with QEMU](#booting-with-qemu)

## Read First

This project is based on PetaLinux 2020.1. 
This seems to build a flawed FSBL by default, so a 2019.1 derived FSBL is provided precompiled.

## Development

The WIB firmware is several different projects, including an Ultrascale+ 
firmware, a Linux kernel and distribution, and the software for the WIB.

### Getting Started

1. Install Vivado 2020.1
2. Clone this repository
3. Inititialize the `wib_sim` submodule with `git submodule update --init`
4. Synthesize the Vivado project contained in the submodule `wib_sim`
5. Follow remaining instructions to generate a linux system and bootable SD image for the WIB

### Organization

* The `wib_sim` project supplies the firmware implementation and Vivado hardware definition.
* An older reference firmware is contained in `reference_fw`
* A PetaLinux project to build the Linux system is in `linux-*`
* The `wib_server` software and test utilities are in `sw`

## Building from Scratch

Building the project is broken into stages:

1. Generate the Ultrascale+ bitstream and hardware definition with Vivado
2. Export hardware definition (.xsa) to the PetaLinux project for the WIB.
3. Configure and build the PetaLinux distribution for the WIB.
4. Create a bootable disk image for the WIB SD card boot mode.

If an earlier stage is modified, later stages typically need to be rerun.

### Generate bitstream

1. Generate bitstream in Vivado.
2. Export bitstream `.bit` (`File`->`Export`->`Export Bitstream File`) to top level of this respository.

#### Quickly updating the bitstream

Consider using the FPGA manager included with the linux kernel on the WIB, which can load a bitstream on the fly.
No further steps are required, though the FPGA manager loaded bitstream will not be persistent across power cycles.

1. Copy your `wib_top.bit` to `/lib/firmware/` on a booted WIB using e.g. `rsync`.
2. Write `"0"` to the sysfs `fpga_manager` utility to prepare it for a new bitstream: `echo "0" > /sys/class/fpga_manager/fpga0/flags`
3. Write the filename of your bitstream (really, a path relative to and within `/lib/firmware/`) to the `fpga_manager` utility, and it will be loaded onto the FPGA: `echo "wib_top.bit" > /sys/class/fpga_manager/fpga0/firmware`
4. A message in the system log should confirm the bitstream was loaded. See [Xilinx docs](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841645/Solution+Zynq+PL+Programming+With+FPGA+Manager#SolutionZynqPLProgrammingWithFPGAManager-Usingsysfsinterface) for more information.

### Export hardware definition

Only necessary if block diagram has changed:

1. Export hardware (`File`->`Export`->`Export Hardware`).
2. Copy the generated `.xsa` to the top level directory in this repository.

### Create PetaLinux system

PetaLinux 2020.1 is required to build the software for the root filesystem 
image and the kernel to boot the WIB. 

You can either build the Docker image provided in `linux-2020.1/petalinux-2020.1` and 
use that environment, or install the packages listed in the `Dockerfile` on a 
machine with PetaLinux 2020.1 already installed. See the 
[container readme](linux/petalinux-2020.1/README.md) for further instructions.

Perform only step 5 if you only want to update the FPGA bitstream. The generated
files can be copied to the SD card boot partition.

1. `cd` into the `linux-2020.1/` folder, which is a Petalinux project.
2. For a new repository, or if block diagram has changed, run `petalinux-config --get-hw-description=../` ensuring that the hardare definition `../*.xsa` is up to date.
3. `petalinux-config` will rewrite the u-boot configuration in a way that probably won't build. Consider reverting those changes with `git checkout project-spec/configs/u-boot-xlnx/config.cfg` unless you know what you are doing. 
4. For a new repository, any changes to the wib software, or if block diagram has changed, run `petalinux-build` to build the linux system and bootloader into `images/linux/`. This can take a long time, but caches build progress for future builds.
5. Run `./make_bootloader_2019fsbl.sh` to generate required boot files (`BOOT.BIN`,`image.ub`,`boot.scr` in `images/linux/`) and a tar of the boot filesystem in `images/linux/bootfs.tar.gz`.

### Creating/Updating boot media

The `linux/make_sd_image.sh` script uses `mtools` and `losetup` to create a
`rootfs.img` file that can be copied to an SD card and boot the WIB. 

1. Ensure your `BOOT.BIN` and `image.ub` files are up-to-date and that `petalinux-build` has been run recently.
2. `cd` into the `linux/` folder.
3. Run `./make_sd_image.sh` to create `../rootfs.img`
4. Assuming your SD card is `/dev/sdX`, run `sudo dd if=../rootfs.img of=/dev/sdX bs=256M status=progress`
5. Run `sync` to ensure the data is written to disk.
6. The SD card is ready to boot the WIB.

#### Alternative method: filesystem tars

The tar files `bootfs.tar.gz` and `rootfs.tar.gz` in `images/linux/` can be used to populate preformatted boot media.
The first partition must be marked bootable and be formatted with a FAT filesystem containing the bootloader, with the linux ext4 filesystem on a second partition

#### Alternative method: `wib_client` update

The `wib_client` can perform a full update of a live system running `wib_server` using its update command, which takes the filesystem tars as inputs.

## Test drive the linux system with QEMU

With PetaLinux's QEMU one can test the boot process before deploying to 
hardware. This also provides an environment to test software on the linux system
without hardware, but note that QEMU does not by default simulate the PL or even
all of the standard Zynq Ultrascale+ hardware (e.g. ethernet).

You can either build the Docker image provided in `linux/petalinux-2020.1` and 
use that environment, or install the packages listed in the `Dockerfile` on a 
machine with PetaLinux 2020.1 already installed. See the
[container README.md](linux-2020.1/petalinux-2020.1/README.md) for further instructions.

### Initial setup

You will need a single file from the `ZCU 102` Board Support Package (BSP) 
from Xilinx `pmu_rom_qemu_sha3.elf` which must be extracted to 
`linux/pre-built/linux/images/`.

You can find the BSP here:
https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools.html

This is really a `.tar.gz` archive, and the file is at: `xilinx-zcu102-2020.1/pre-built/linux/images/`

### Booting with QEMU

From the `linux/` directory, boot the image with:

`petalinux-boot --qemu --uboot --qemu-args "-drive file=../rootfs.img,if=sd,format=raw,index=1"`

QEMU will first launch `uboot` in a virtual Ultrascale+ device, which will read 
the provided SD card image `../rootfs.img` and start the Linux kernel on the 
boot partition.

Additional options can be passed to QEMU with `--qemu-args` or directly to
`petalinux-boot` to modify the simulated hardware.
