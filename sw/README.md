# WIB Software

This is usually built by the Peta Linux project and included in the WIB root
filesystem image, however for testing `wib_server` this can be compiled on 
any system with the included `Makefile`.

## Client / Server model

The parts of this project that access the WIB hardware runs on the WIB itself,
as the `wib_server` program. This is started by the init system after the kernel
loads on the WIB, and provices a `REP` ZeroMQ server that impelements a protocol
defined in `src/wib.proto` allowing for remote control of the WIB. 

A reference remote control program, `wib_client`, is included, which has a 
Read-Evaluate-Print Loop (REPL) implemented with readline. This provides a 
command line interface to control the WIB, which can either be run locally on 
the WIB itself, or remotely by specifying the WIB's IPv4 address.

## Connecting to cocotb simulation

The `Makefile` can build a `simulation` target which builds fake hardware 
interfaces into the `wib_server` software. This fake interface is a ZeroMQ 
socket which sends register read/write commands to a cocotb simulation of 
the WIB firmware in real time.
