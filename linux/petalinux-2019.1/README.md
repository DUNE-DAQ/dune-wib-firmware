# PetaLinux 2019.1 Docker Container

This containerizes a petalinux environment for building the WIB root filesystem 
including the onboard WIB software and Linux kernel. 

## Usage

1. Download `petalinux-v2019.1-final-installer.run` from Xilinx and place it in this folder.
2. Build the docker container `docker build --build-arg INSTALLER=petalinux-v2019.1-final-installer.run -t petalinux:2019.1 .`
3. Start the docker container `docker run -v /path/to/dune-wib-firmware/:/home/plnx/project/ -it petalinux:2019.1 /bin/bash`
