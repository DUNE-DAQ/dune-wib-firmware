# DUNE WIB Firmware

Central repository for development of DUNE WIB firmware.

## Development

### Getting Started

1. Install Vivado 2019.1
2. Clone this repository
3. Source `settings64.sh` from the Vivado install
4. Run `./init.sh` to create `DUNE_WIB` Vivado project

### Organization

* Project source (Verilog or VHDL) organized under `src/`
* Imported IPs should go in `ip/` (setup as Vivado IP Repository)
* Place constraints in `constr/`

### Version Control

Xilinx recommends using version control on `.tcl` scripts, which are used to generate a project, rather than the project itself.
This methodology is followed here, and all sources are outside of the project directory hierarchy, and should be committed to this repository when changed.
All new files should be added as **remote** to the project, and stored according to the Organization section.
As a result, any changes to the project will only be local, unless the `DUNE_WIB.tcl` file is recreated and committed to the repository.
To do this from Vivado, 
1. `File`->`Project`->`Write Tcl...`
2. Choose the `DUNE_WIB.tcl` file in this repository.
3. Ensure the following are checked
    * "Copy sources to new project"
    * "Recreate Block Diagrams using Tcl"
4. Commit the new `.tcl` to the repository.

## Generating FPGA Bitstream and Linux Boot Image

### Generate bitstream

1. Generate bitstream in Vivado.
2. Export bitstream (`File`->`Export`->`Export Bitstream File`) to `DUNE_WIB.bit`.

### Export hardware definition

Only necessary if block diagram has changed:

1. Export hardware (`File`->`Export`->`Export Hardware`).
2. Copy `DUNE_WIB/DUNE_WIB.sdk/DUNE_WIB.hdf` to `DUNE_WIB.hdf`.

### Create PetaLinux images

PetaLinux 2019.1 from Xilinx should be installed. Source the `settings.sh` file from that install. 
Start at step 4 if you only want to update the FPGA bitstream.

1. `cd` into the `linux/` folder.
2. For a new repository, or if block diagram has changed, run `petalinux-config --get-hw-description=../` ensuring that the hardare definition `../DUNE_WIB.hdf` is up to date.
3. For a new repository, or if block diagram has changed, run `petalinux-build` to build the linux system.
4. Run `./rebuild_boot_image.sh` to generate `../BOOT.BIN` and `../image.ub`.
5. Copy (at least) `BOOT.BIN` to SD card boot partition for a new bitstream (`image.ub` as well, to update kernel).
6. Reboot the WIB.
