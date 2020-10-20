# WIB Software

This serves as documentation for the WIB software.

The software is usually built by the Peta Linux project and included in the WIB 
root file system image, however for testing `wib_server` this can be compiled on 
any system with the included `Makefile`. The `wib_client` is a standalone
program, and can be used to remote control a WIB running `wib_server`.

- [WIB Software](#wib-software)
  * [Client / Server Overview](#client---server-overview)
  * [Basic Usage](#basic-usage)
    + [`wib_server`](#-wib-server-)
    + [`wib_client`](#-wib-client-)
    + [WIB scripts](#wib-scripts)
      - [Usage](#usage)
      - [Syntax](#syntax)
    + [DAQ spy buffer readout](#daq-spy-buffer-readout)
    + [Updating the WIB](#updating-the-wib)
  * [Connecting to cocotb simulation](#connecting-to-cocotb-simulation)


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
    Initialize the WIB hardware (TBD)
  script filename
    Run a WIB script (local file will be sent, otherwise filename is remote in /etc/wib/)
  daqspy filename
    Read 1MB from each daq spy buffer and write the 2MB data to filename
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
    Closes the command interfac
```

### WIB scripts

The `wib_server` has a simple scriptable interface for performing bulk 
configuration. These scripts are limited to basic memory, i2c, coldata i2c, and
coldata fastcommand operations.

#### Usage

The `wib_client` has a `script` command to launch scripts by name. The 
`scripts` directory will be installed to the WIB in `/etc/wib/` and all of these
are valid names. Additionally, if the argument to `script` is matches a local
file, this will be copied to the WIB prior to running. Note that `run` commands
in a script must be remote files. Full paths to files can also be given, as the
`wib_server` will first search `/etc/wib/` for a matching script.

#### Syntax

Scripts are a series of newline-separated commands with space-separated 
arguments. Lines starting with `#` will be ignored, as will empty lines. See 
the [scripts](scripts) directory for example scripts.

The following commands are valid:

- `i2c` with arguments `bus chip register data [data...]` - Perform an onboard I2C write.
  * An `i2c_reg_write` is performed if only one `data` is specified, or an `i2c_block_write` if there are multiple data (see `i2c.h`).
  * `bus` can be either `sel` or `pwr` to correspond to the WIB selectable I2C bus (see `wib.h`) or power I2C bus.
  * `chip` `register` and `data` are all hex values and single bytes.  
- `i2c cd` with arguments `femb coldata chip page addr data` - Perform a COLDATA I2C write.
  *  `cd` is a literal value (a specific `bus` above, but with different syntax).
  * `femb` should be 0-3 and `coldata` should be 0-1 to specify a specific COLDATA.
  *  `chip`, `page`, `addr`, and `data` are all hex values and single bytes.
- `mem` with arguments `addr value [mask]` - Perform a write to the WIB's address space
  * If `mask` is specified, the `addr` will first be read, and only the bits in `mask` will be set from `value`.
  * `addr`, `value`, and `mask` (if specified) are all 32 bit hex values.
  * This always performs a 32bit write.
- `fast` with arguments `cmd` - Perform a fast command on all COLDATA chips
  * `cmd` can be one of: `reset`, `act`, `sync`, `edge`, `idle`, `edge_act`
- `delay` with arguments `micros` - Sleep (at least) a specified number of microseconds
- `run` with argument `script` - Execute another script
  * `script` must either be a full path on the WIB, or a script in `/etc/wib/` on the WIB

### DAQ spy buffer readout

The `wib_client` can start a spy buffer acquisition on the WIB to save 1 MB of 
data from each spy buffer with the `daqspy` command. This will be returned to 
the `wib_client` and saved in the specified file. Both buffers will be returned
regardless of success, as success only indicates that the spy buffer filled 
within at 10ms timeout period.

### Updating the WIB

The `wib_client` update command can take a tar archive of the root and boot
filesystems, transfer them to the WIB, and untar over the active system. You
should immediately `reboot` after a successful `update` to deploy the new 
system.

## Connecting to cocotb simulation

The `Makefile` can build a `simulation` target which builds fake hardware 
interfaces into the `wib_server` software. This fake interface is a ZeroMQ 
socket which sends register read/write commands to a cocotb simulation of 
the WIB firmware in real time.
