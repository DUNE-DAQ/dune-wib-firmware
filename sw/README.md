# WIB Software

This is usually built by the Peta Linux project and included in the WIB root
filesystem image, however for testing `wib_server` this can be compiled locally.

## Connecting to cocotb simulation

The `Makefile` can build a `simulation` target which builds fake hardware 
interfaces into the `wib_server` software. This fake interface is a ZeroMQ 
socket which sends register read/write commands to a cocotb simulation of 
the WIB firmware in real time.
