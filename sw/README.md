# WIB Software

This is usually built by the Peta Linux project and included in the WIB root
file system image, however for testing `wib_server` this can be compiled on 
any system with the included `Makefile`.

## Client / Server Overview

The parts of this project that access the WIB hardware runs on the WIB itself,
as the `wib_server` program. This is started by the init system after the kernel
loads on the WIB, and provides a `REP` ZeroMQ server that implements a protocol
defined in `src/wib.proto` allowing for remote control of the WIB. 

A reference remote control program, `wib_client`, is included, which has a 
Read-Evaluate-Print Loop (REPL) implemented with readline. This provides a 
command line interface to control the WIB, which can either be run locally on 
the WIB itself, or remotely by specifying the WIB's IPv4 address.

## Basic Usage

### `wib_server`

The `wib_server` takes no arguments, and is usually started on boot by the init
system with `/etc/rc5.d/wib_init.sh` derived from `extras/wib_init.sh`.
You should not have to run this directly.

### `wib_client`

Usage: `./wib_client [-w ip] [cmd]`

The `wib_client` can either be run on the WIB without specifying an IP (connects
to `wib_server` listening on 127.0.0.1), or run remotely by specifying the WIBs
IP address.

The `cmd` option can be omitted, which will open a command interface with a `>>`
prompt. Otherwise a single command and its arguments can be passed directly.

The `help` command for `wib_server` will provide a list of commands and a brief
description of each:

```
Available commands:
  reboot
    Reboot the WIB
  initialize
    Initialize the WIB hardware
  peek addr
    Read a 32bit value from WIB address space
  poke addr value
    Write a 32bit value to WIB address space
  cdpeek femb_idx cd_idx chip_addr reg_page reg_addr
    Read a 8bit value from COLDATA I2C address space
  cdpoke femb_idx cd_idx chip_addr reg_page reg_addr data
    Write a 8bit value to COLDATA I2C address space
  cdfastcmd cmd
    Send the fast command cmd to all coldata chips
  update root_archive boot_archive
    Deploy a new root and boot archive to the WIB
  exit
    Closes the command interface
```

## Connecting to cocotb simulation

The `Makefile` can build a `simulation` target which builds fake hardware 
interfaces into the `wib_server` software. This fake interface is a ZeroMQ 
socket which sends register read/write commands to a cocotb simulation of 
the WIB firmware in real time.
