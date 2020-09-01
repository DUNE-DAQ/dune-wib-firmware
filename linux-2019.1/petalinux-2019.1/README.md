# PetaLinux 2019.1 Docker Container

This containerizes a petalinux environment for building the WIB root filesystem 
including the WIB server software and Linux kernel, creating a portable build
system usable on any host that can run Docker containers.

## Requirements

1. Docker must be available on a host system, and you must have permission to build and start containers. (You may need root access.)
2. You will need to download the [linux64 PetaLinux distribution installer from Xilinx](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools/2019-1.html) to install it in the container.

## Basic Usage

1. Download [`petalinux-v2019.1-final-installer.run`](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools/2019-1.html) from Xilinx and place it in this folder.
2. Build the docker container `docker build --build-arg INSTALLER=petalinux-v2019.1-final-installer.run -t petalinux:2019.1 .`
3. Start the docker container `docker run -v /path/to/dune-wib-firmware/:/home/plnx/project/ -it petalinux:2019.1 /bin/bash`
