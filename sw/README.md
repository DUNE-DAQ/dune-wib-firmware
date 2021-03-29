# WIB Software

This serves as documentation for the WIB software stack.

This softare primarily runs on the WIB to perform the critical tasks of 
configuring, controlling, and monitoring the frontend and firmware on the WIB.

The software is usually built by the Peta Linux project and included in the WIB 
root file system image, however for testing `wib_server` this can be compiled on 
any system with the included `Makefile`. The `wib_client` is a standalone
program, and can be used to remote control a WIB running `wib_server`. The 
`wib_scope.py` program is a python3 Qt GUI diagnostic interface to the WIB.
These are described in the following 

- [WIB Software](#wib-software)
  * [Client Server Overview](#client-server-overview)
  * [Dependencies and Building](#dependencies-and-building)
  * [Software Components](#software-components)
    + [wib_server](#wib_server)
    + [wib_client.py](#wib_clientpy)
    + [spy_dump](#spy_dump)
    + [sync_fake_time](#sync_fake_time)
    + [wib_scope.py](#wib_scopepy)
      - [Help](#help)
      - [Configuration](#configuration)
      - [Pulser](#pulser)
      - [Acquire](#acquire)
      - [Plotting data](#plotting-data)
      - [Creating useful display](#creating-useful-display)
    + [femb_diagnostic.py](#femb_diagnosticpy)
    + [femb_linearity.py](#femb_linearitypy)
    + [wib_mon.py](#wib_monpy)
    + [wib_config.py](#wib_configpy)
    + [wib_power.py](#wib_powerpy)
    + [wib_power_conf.py](#wib_power_confpy)
  * [Functionality Overview](#functionality-overview)
    + [Low level functions](#low-level-functions)
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

## Dependencies and Building

The C++ components (`wib_client` and `wib_server`) both require C++ development 
packages and runtime libraries for:

* protobuf
* zeromq
* cppzmq
* readline
* i2c-tools
* rsync (`wib_server` runtime only)
* tar (`wib_server` runtime only)

See the [bitbake for the PetaLinux distribution](../linux-2020.1/project-spec/meta-user/recipes-apps/wib/wib.bb)
for more installation instructions. Build with `make` or `make simulation`
depending on whether a real or simulated hardware interface is desired in 
`wib_server`. (NOTE: do not switch between these without `make clean`!)

The python components (`wib_scope.py`) requires the following python3 packages:

* protobuf
* pyzmq
* numpy
* matplotlib
* pyqt5 (pyqt4 may work)

Generate the python protobuf library with `make python`.

## Software Components

### wib_server

The `wib_server` takes no arguments, and is usually started on boot by the init
system with `/etc/rc5.d/wib_init.sh` derived from `extras/wib_init.sh`.
You should not have to run this directly.

When running on the WIB, the `wib_server` acts as an interface to the WIB
hardware. It will listen on TCP port `1234` for `Command` protobuf messages 
defined in [`wib.proto`](src/wib.proto). The `wib_server` will respond with 
messages as described in the comments of [`wib.proto`](src/wib.proto).

### wib_client.py

Usage: `wib_client.py [-h] [-w WIB_SERVER] [command] [args ...]`

The WIB client can either be run on the WIB without specifying an IP (connects
to `wib_server` listening on 127.0.0.1), or run remotely by specifying the WIBs
IP address.

The `cmd` option can be omitted, which will open a command interface with a `>>`
prompt. Otherwise a single command and its arguments can be passed directly.

The `help` command for `wib_client.py` will provide a list of commands and a
brief description of each. Please see the output of the program for the most 
up-to-date listing. Use `help [command]` for argument details for a specific 
command.

```
commands:
    reboot              Reboot the WIB
    log                 Return or control the wib_server log
    fw_timestamp        Return firmware version timestamp
    sw_version          Return software build version
    timing_reset        Reset the timing endpoint
    timing_status       Return the status of the timing endpoint
    script              Run a WIB script
    config              Send frontend configuration to the WIB
    calibrate           Run the ADC calibration routine
    daqspy              Read 1MB from each daq spy buffer and write the (up to) 2MB binary data
    peek                Read a 32bit value from WIB address space
    poke                Write a 32bit value to WIB address space
    cdpeek              Read a 8bit value from COLDATA I2C address space
    cdpoke              Write a 8bit value to COLDATA I2C address space
    cdfastcmd           Send the fast command cmd to all coldata chips
    update              Deploy a new root and boot archive to the WIB
    exit                Closes the command interface
    help                Show help message, optionally for a subcommand
```

There is also a C++ version of this command, `wib_client`, with a reduced set
of functionality for environments without Python or as a C++ reference for 
communicating with the WIB via its ZMQ socket.

### spy_dump

Usage: `./spy_dump [daqspy_binary]`

This utility will emit the contents of a `daqspy` binary created from 
`wib_client` as human readable hex values for debugging.

### sync_fake_time

Usage: `./sync_fake_time hextime [wib_ip] [wib_ip] ...`

This utility will attempt to synchronize the fake timestamp generator in the 
WIB firmware over the network with minimal latency. Multiple WIB IPs can be 
specified (no limit) along with an initial timestamp (in hex notation) common
to all. Do not expect better than few-ms synchronization using this method. 
Only useful prior to a fully functional timing endpoint in the WIB.

### wib_scope.py

This is a pyqt5 GUI interface to the WIB frontend configuration, calibration,
and daq spy buffer readout functionality. Build the protobuf python library 
for the WIB with `make python` before running it.

```
usage: wib_scope.py [-h] [--wib_server WIB_SERVER] [--config CONFIG] [--rows ROWS] [--cols COLS] [--layout LAYOUT]

Visually display data from a WIB

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --config CONFIG, -C CONFIG
                        WIB configuration to load [defaults]
  --rows ROWS, -r ROWS  Rows of plots [1]
  --cols COLS, -c COLS  Columns of plots [1]
  --layout LAYOUT, -l LAYOUT
                        Load a saved layout
```

#### Configuration

The `Configure` button on the GUI will load the front end configuration 
described in the `config` JSON document. See the comments in the `ConfigureWIB`
message of [`wib.proto`](src/wib.proto).

#### Pulser

The `Enable Pulser` or `Disable Pulser` buttons will start or stop the COLDATA
calibration strobe. Assumes the strobe parameters have already been configured.

#### Acquire

The `Acquire` button will read the contents of the WIB daq spy buffer, deframe
the data on the WIB, and transfer it to the `wib_scope.py` program for
plotting.

#### Plotting data

After clicking a plot, a set of configuration options will appear in the lower
left of that plot's axes. The `Signals` button allow channels from the WIB and
various features (such as performing a FFT or baseline correction) to be 
changed. The remaining buttons have informative tooltip text.

#### Creating useful display

The buttons along the top relate to the number of plots visible on the display
and their configurations. `Reshape` will change the size of the grid of plots.
`Load Layout` and `Save Layout` will reload or save the window setup, but not
the data.

After creating a grid, select the signals and featureson each plot after 
clicking it.

### femb_diagnostic.py

This is a pyqt5 GUI interface to produce diagnostic plots for FEMBs attached
to a WIB. It has similar functionality to [wib_scope.py](#wib_scopepy) except
that the data displayed includes:
* A 2D histogram of ADC counts per channel on FEMB 0
* The mean and RMS ADC values for each channel on FEMB 0
* The power spectrum (FFT) of each channel on FEMB 0 as a 2D histogram

By default this utility loads a configuration with only the selected FEMB. You
must press `Configure` to load this onto the WIB, similar to `wib_scope.py`

```
usage: femb_diagnostic.py [-h] [--wib_server WIB_SERVER] [--femb FEMB] [--cold] [--test] [--config CONFIG] [--grid] [--save_to SAVE_TO]

Visually display diagnostic data plots from a FEMB on a WIB

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --femb FEMB, -f FEMB  FEMB index to display 0-3 [0]
  --cold, -c            The FEMBs will load the cold configuration with this option [default: warm]
  --test, -t            ADCs will digitize a test pattern instead of digitized data [default: off]
  --config CONFIG, -C CONFIG
                        WIB configuration to load [default: generated with --femb and --cold]
  --grid, -g            Split Mean/RMS into separate plots for a 2x2 grid
  --save_to SAVE_TO, -s SAVE_TO
                        Path to save plots to (only last plotted saved)
```

### femb_linearity.py

This is a command line utility to acquire pulser data from one or several FEMBs
at pulser DAC settings 0,5,10,15,20. It can additionally analyze the acquired
data with a peakfinding algorithm to produce plots of peak ADC count vs pulser
DAC setting per channel. This is intended to show that FEMBs are in a minimally
working state.

```
usage: femb_linearity.py [--help [subcommand]] {acquire,analyze} ...

Acquire pulser data from FEMBs and/or perform linearity test

optional arguments:
  --help [subcommand], -h [subcommand]
                        show help message and exit

subcommands:
  {acquire,analyze}     subcommand help
    acquire             Acquire data from a WIB using the spy buffer and save to HDF5 file
    analyze             Analyze HDF5 file to find pulse peak values and produce linaerity plots
```

### wib_mon.py

This is another pyqt5 GUI interface to the WIBs onboard sensors. It can display
current, voltage, and temperatures from the integrated I2C sensors. Optionally
this will print to the command line instead of opening a GUI for archival
purposes.

```
usage: wib_mon.py [-h] [--wib_server WIB_SERVER] [--cli]

Visually display monitoring info from a WIB

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --cli, -c             Query sensors and print to CLI only
```

### wib_config.py

This is a command line utility for loading a JSON config document specifying 
frontend electronics settings into the WIB. The format for this is identical to
that [used by wib_scope.py](#configuration).

```
usage: wib_config.py [-h] [--wib_server WIB_SERVER] [--config CONFIG]

Send a configuration json document to a WIB

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --config CONFIG, -C CONFIG
                        WIB configuration to load [defaults]
```

### wib_power.py

This is a command line utility for setting the power state for the FEMBs
attached to a particular WIB. Currently no other utilities have this 
functionality implemented, though it is achieved with a ZMQ message like all
other functionality. The power state is specified for each FEMB, and the WIB
will run the appropriate sequence of commands to achieve the desired state.

This will load the power configuration for the FEMBs stored on the WIB. To first
change this configuration, use `wib_power_conf.py`.

```
usage: wib_power.py [-h] [--wib_server WIB_SERVER] [--cold] [--stage {full,pre,post}] {on,off} {on,off} {on,off} {on,off}

Change the FEMB power state on a WIB

positional arguments:
  {on,off}              Power FEMB_0
  {on,off}              Power FEMB_1
  {on,off}              Power FEMB_2
  {on,off}              Power FEMB_3

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --cold, -c            The FEMBs will load the cold configuration with this option [default: warm]
  --stage {full,pre,post}, -s {full,pre,post}
                        Run full power ON sequence or pre/post ADC synchronization stages [default: full]
```

### wib_power_conf.py

This utility will modify the power configuration for the FEMBs that is stored on
the WIB. This will first power off the FEMBs if they are on, and then program
the FEMB regulators. Use `wib_power.py` to then turn the FEMBs on with the new
configuration.

```
usage: wib_power_conf.py [-h] [--wib_server WIB_SERVER] [--dc2dc-o1 DC2DC_O1] [--dc2dc-o2 DC2DC_O2] [--dc2dc-o3 DC2DC_O3] [--dc2dc-o4 DC2DC_O4] [--ldo-a0 LDO_A0] [--ldo-a1 LDO_A1]

Change the FEMB voltages on a WIB (first turns FEMBs OFF)

optional arguments:
  -h, --help            show this help message and exit
  --wib_server WIB_SERVER, -w WIB_SERVER
                        IP of wib_server to connect to [127.0.0.1]
  --dc2dc-o1 DC2DC_O1, --o1 DC2DC_O1
                        DC2DC O1 output voltage [4.0V]
  --dc2dc-o2 DC2DC_O2, --o2 DC2DC_O2
                        DC2DC O2 output voltage [4.0V]
  --dc2dc-o3 DC2DC_O3, --o3 DC2DC_O3
                        DC2DC O3 output voltage [4.0V]
  --dc2dc-o4 DC2DC_O4, --o4 DC2DC_O4
                        DC2DC O4 output voltage [4.0V]
  --ldo-a0 LDO_A0, --a0 LDO_A0
                        LDO A0 output voltage [2.5V]
  --ldo-a1 LDO_A1, --a1 LDO_A1
                        LDO A1 output voltage [2.5V]
```

## Functionality Overview

### Low level functions

The `wib_server` will respond to requests to read and write onboard memory
and also the frontend I2C bus.

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
- `cd-i2c` with arguments `femb coldata chip page addr data` - Perform a COLDATA I2C write.
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

The WIB `wib_server` can also deframe this data, and the `wib_scope.py` 
program can visualize it.

### Updating the WIB

The `wib_client` update command can take a tar archive of the root and boot
filesystems, transfer them to the WIB, and untar over the active system. You
should immediately `reboot` after a successful `update` to deploy the new 
system.

## Connecting to cocotb simulation

The `Makefile` can build a `simulation` target which builds fake hardware 
interfaces into the `wib_server` software. This fake interface is a ZeroMQ 
socket which sends register read/write commands to a cocotb simulation of 
the WIB firmware in real time (if such a thing were to exist). 

Without an AXI simulation, this just prints register reads and writes that 
would have happened instead of performing them.
