# DUNE WIB Firmware

Central repository for development of DUNE WIB firmware.

## Getting Started

1. Install Vivado 2019.1
2. Clone this repository
3. Source settings64.sh from the Vivado install
4. Run `./init.sh` to create `DUNE_WIB` Vivado project

## Organization

* Project source (Verilog or VHDL) organized under `src/`
* Imported IPs should go in `ip/` (setup as Vivado IP Repository)
* Place constraints in `constr/`

## Version Control

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
