A lot of these scripts were transcribed from very early debugging scripts so 
that they can be invoked remotely with the wib_server's script command. Most
are no longer used in standard running, but may be useful for debugging.

The important exception to this are the PLL/clock/timing config scripts, which
are invoked at the first config after power is turned ON:
* `conf_pll_timing` - calls the rest
* `cdr_reset` - resets clock&data recovery chip
* `si5344_62p5mhz_config` - configures the PLL chip (a 50MHz versions exists)
* `pll_sticky_clear` - resets status registers
* `ept_reset` - resets the timing endpoint

Other scripts may have unintended side effects.
