#!/usr/bin/env python3

import os
import sys
import time
import shlex
import pickle
import readline
import argparse

from wib import WIB
import wib_pb2 as wibpb

parser = argparse.ArgumentParser(description='Python version of the wib_client utility')
parser.add_argument('-w','--wib_server',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
parser.add_argument('command',nargs='?',help='Execute a single command (opens CLI if omitted; help command for more info)')
parser.add_argument('args',nargs='*',help='Optional arguments to command')

commands = argparse.ArgumentParser(add_help=False,prog='')
class CommandException(Exception):
    def __init__(self,message,parser=None):
        self.message = message
        self.parser = parser
def custom_exit(*args, message=None, **kwargs):
    raise CommandException(message)
def custom_error(message, *args, **kwargs):
    raise CommandException(message)
commands.exit = custom_exit
commands.error = custom_error
sub = commands.add_subparsers(title='subcommands',help='subcommand help')

def bind_parser(parser,func):
    def custom_exit(*args, message=None, **kwargs):
        raise CommandException(message,parser=parser)
    def custom_error(message, *args, **kwargs):
        raise CommandException(message,parser=parser)
    parser.exit = custom_exit
    parser.error = custom_error
    parser.set_defaults(func=func)

reboot_parser = sub.add_parser('reboot',help='Reboot the WIB',add_help=False)
def reboot(args):
    req = wibpb.Reboot()
    rep = wibpb.Empty()
    wib.send_command(req,rep)
    print('Rebooting...')
bind_parser(reboot_parser,reboot)

log_parser = sub.add_parser('log',help='Return or control the wib_server log',add_help=False)
log_parser.add_argument('action',nargs='?',choices=['clear','boot'],help='Optional log action (returns log if omitted)')
def log(args):
    req = wibpb.LogControl()
    req.return_log = args.action == None
    req.boot_log = args.action == 'boot'
    req.clear_log = args.action == 'clear'
    rep = wibpb.LogControl.Log()
    wib.send_command(req,rep)
    print(rep.contents.decode('ascii'),end='')
bind_parser(log_parser,log)

fw_timestamp_parser = sub.add_parser('fw_timestamp',help='Return firmware version timestamp',add_help=False)
def fw_timestamp(args):
    req = wibpb.GetTimestamp()
    rep = wibpb.GetTimestamp.Timestamp()
    wib.send_command(req,rep)
    print('fw_timestamp code: 0x%08X'%rep.timestamp);
    print('decoded: %i/%i/%i %i:%i:%i'%(rep.year,rep.month,rep.day,rep.hour,rep.min,rep.sec));
bind_parser(fw_timestamp_parser,fw_timestamp)

sw_version_parser = sub.add_parser('sw_version',help='Return software build version',add_help=False)
def sw_version(args):
    req = wibpb.GetSWVersion()
    rep = wibpb.GetSWVersion.Version()
    wib.send_command(req,rep)
    print('sw_version: %s'%rep.version);
bind_parser(sw_version_parser,sw_version)

timing_reset_parser = sub.add_parser('timing_reset',help='Reset the timing endpoint',add_help=False)
def timing_reset(args):
    req = wibpb.ResetTiming()
    rep = wibpb.GetTimingStatus.TimingStatus()
    wib.send_command(req,rep)
    wib.print_timing_status(rep)
bind_parser(timing_reset_parser,timing_reset)

timing_status_parser = sub.add_parser('timing_status',help='Return the status of the timing endpoint',add_help=False)
def timing_status(args):
    req = wibpb.GetTimingStatus()
    rep = wibpb.GetTimingStatus.TimingStatus()
    wib.send_command(req,rep)
    wib.print_timing_status(rep)
bind_parser(timing_status_parser,timing_status)

script_parser = sub.add_parser('script',help='Run a WIB script',add_help=False)
script_parser.add_argument('filename',help='local file will be sent, otherwise filename is remote in /etc/wib/ on the WIB')
def script(args):
    req = wibpb.Script()
    rep = wibpb.Status()
    if os.path.exists(args.filename):
        print('Executing local script...')
        with open(args.filename,'rb') as fin:
            req.script = fin.read()
        req.file = False
    else:
        print('Executing remote script...')
        req.script = bytes(args.filename, 'utf-8')
        req.file = True
    wib.send_command(req,rep)
    print('Successful:',rep.success)
bind_parser(script_parser,script)

config_parser = sub.add_parser('config',help='Send frontend configuration to the WIB',add_help=False)
config_parser.add_argument('filename',help='JSON config file')
def config(args):
    wib.config(args.filename)
bind_parser(config_parser,script)

log_parser = sub.add_parser('calibrate',help='Run the ADC calibration routine',add_help=False)
def log(args):
    req = wibpb.Calibrate()
    rep = wibpb.Status()
    wib.send_command(req,rep)
    print(rep.extra.decode('ascii'),end='')
    print('Successful:',rep.success)
bind_parser(log_parser,log)

daqspy_parser = sub.add_parser('daqspy',help='Read 1MB from each daq spy buffer and write the (up to) 2MB binary data',add_help=False)
daqspy_parser.add_argument('filename',help='Output file for binary data')
daqspy_parser.add_argument('buffers',nargs='?',choices=['buf0','buf1','both'],default='both',help='Select specific buffers [both]')
daqspy_parser.add_argument('--cmd',type=int,default=0,help='TLU commmand to use as trigge, or 0 for software trigger [0]')
daqspy_parser.add_argument('--rec',type=int,default=180360,help='Record time after trigger in 4.158ns ticks [180360 ~750us]')
daqspy_parser.add_argument('--timeout',type=int,default=60000,help='Trigger timeout time in ms [60000 ~1min]')
def daqspy(args):
    req = wibpb.ReadDaqSpy()
    req.buf0 = args.buffers in ['buf0','both']
    req.buf1 = args.buffers in ['buf1','both']
    req.trigger_command = args.cmd
    req.trigger_rec_ticks = args.rec
    req.trigger_timeout_ms = args.timeout
    rep = wibpb.ReadDaqSpy.DaqSpy()
    wib.send_command(req,rep)
    print('Successful:',rep.success)
    with open(args.filename,'wb') as fout:
        fout.write(rep.buf0)
        fout.write(rep.buf1)
bind_parser(daqspy_parser,daqspy)

peek_parser = sub.add_parser('peek',help='Read a 32bit value from WIB address space',add_help=False)
peek_parser.add_argument('addr',type=lambda x: int(x,16),help='Address to read (hex)')
def peek(args):
    req = wibpb.Peek()
    rep = wibpb.RegValue()
    req.addr = args.addr
    wib.send_command(req,rep)
    print('*(0x%016X) -> 0x%08X'%(rep.addr,rep.value))
bind_parser(peek_parser,peek)

poke_parser = sub.add_parser('poke',help='Write a 32bit value to WIB address space',add_help=False)
poke_parser.add_argument('addr',type=lambda x: int(x,16),help='Address to write (hex)')
poke_parser.add_argument('value',type=lambda x: int(x,16),help='Value to write to addr (hex)')
def poke(args):
    req = wibpb.Poke()
    rep = wibpb.RegValue()
    req.addr = args.addr
    req.value = args.value
    wib.send_command(req,rep)
    print('*(0x%016X) <- 0x%08X'%(rep.addr,rep.value))
bind_parser(poke_parser,poke)

cdpeek_parser = sub.add_parser('cdpeek',help='Read a 8bit value from COLDATA I2C address space',add_help=False)
cdpeek_parser.add_argument('femb_idx',type=int,choices=[0,1,2,3],help='FEMB to communicate with [0-3]')
cdpeek_parser.add_argument('coldata_idx',type=int,choices=[0,1],help='COLDDATA chip to communicate with [0-1]')
cdpeek_parser.add_argument('chip_addr',type=lambda x: int(x,16),help='DUNE I2C chip address (hex)')
cdpeek_parser.add_argument('reg_page',type=lambda x: int(x,16),help='DUNE I2C register page (hex)')
cdpeek_parser.add_argument('reg_addr',type=lambda x: int(x,16),help='DUNE I2C register address (hex)')
def cdpeek(args):
    req = wibpb.CDPeek()
    rep = wibpb.CDRegValue()
    req.femb_idx = args.femb_idx
    req.coldata_idx = args.coldata_idx
    req.chip_addr = args.chip_addr
    req.reg_page = args.reg_page
    req.reg_addr = args.reg_addr
    wib.send_command(req,rep)
    print('femb:%i coldata:%i chip:0x%02X page:0x%02X reg:0x%02X -> 0x%02X'%(rep.femb_idx,rep.coldata_idx,rep.chip_addr,rep.reg_page,rep.reg_addr,rep.data))
bind_parser(cdpeek_parser,cdpeek)

cdpoke_parser = sub.add_parser('cdpoke',help='Write a 8bit value to COLDATA I2C address space',add_help=False)
cdpoke_parser.add_argument('femb_idx',type=int,choices=[0,1,2,3],help='FEMB to communicate with')
cdpoke_parser.add_argument('coldata_idx',type=int,choices=[0,1],help='COLDDATA chip to communicate with')
cdpoke_parser.add_argument('chip_addr',type=int,help='DUNE I2C chip address')
cdpoke_parser.add_argument('reg_page',type=int,help='DUNE I2C register page')
cdpoke_parser.add_argument('reg_addr',type=int,help='DUNE I2C register address')
cdpoke_parser.add_argument('data',type=int,help='Data to write over DUNE I2C')
def cdpoke(args):
    req = wibpb.CDPoke()
    rep = wibpb.CDRegValue()
    req.femb_idx = args.femb_idx
    req.coldata_idx = args.coldata_idx
    req.chip_addr = args.chip_addr
    req.reg_page = args.reg_page
    req.reg_addr = args.reg_addr
    req.data = args.data
    wib.send_command(req,rep)
    print('femb:%i coldata:%i chip:0x%02X page:0x%02X reg:0x%02X <- 0x%02X'%(rep.femb_idx,rep.coldata_idx,rep.chip_addr,rep.reg_page,rep.reg_addr,rep.data))
bind_parser(cdpoke_parser,cdpoke)

cdfastcmd_parser = sub.add_parser('cdfastcmd',help='Send the fast command cmd to all coldata chips',add_help=False)
cdfastcmd_parser.add_argument('command',choices=['reset', 'act', 'sync', 'edge', 'idle', 'edge_act'],help='FASTCMD to send')
def cdfastcmd(args):
    fast_cmds = { 'reset':1, 'act':2, 'sync':4, 'edge':8, 'idle':16, 'edge_act':32 }
    req = wibpb.CDFastCmd()
    req.cmd = fast_cmds[args.command]
    rep = wibpb.Empty()
    wib.send_command(req,rep)
    print('Fast command sent')
bind_parser(cdfastcmd_parser,cdfastcmd)

update_parser = sub.add_parser('update',help='Deploy a new root and boot archive to the WIB',add_help=False)
update_parser.add_argument('root_archive',help='Root filesystem archive')
update_parser.add_argument('boot_archive',help='Boot filesystem archive')
def update(args):
    if not os.path.exists(args.root_archive):
        print('Root archive',args.root_archive,'not found. Aborting update.')
        return
    if not os.path.exists(args.boot_archive):
        print('Boot archive',args.boot_archive,'not found. Aborting update.')
        return
    req = wibpb.Update()
    with open(args.root_archive,'rb') as froot:
        req.root_archive = froot.read()
    with open(args.boot_archive,'rb') as fboot:
        req.boot_archive = fboot.read()
    rep = wibpb.Empty()
    print('Sending update command...')
    wib.send_command(req,rep) 
    print('WIB will now update and reboot.')
bind_parser(update_parser,update)

exit_parser = sub.add_parser('exit',help='Closes the command interface',add_help=False)
def exit(args):
    sys.exit(0)
bind_parser(exit_parser,exit)

help_parser = sub.add_parser('help',help='Show help message, optionally for a subcommand',add_help=False)
help_parser.add_argument('command',nargs='?',help='Subcommand to show help for')
def help(args):
    if args.command is None:
        commands.print_help()
    else:
        p = '%s_parser'%args.command
        if p in globals():
            eval('%s.print_help()'%p)
        else:
            print('Unknown command %s'%args.command)
bind_parser(help_parser,help)

def handle_args(args):
    try:
        args = commands.parse_args(args)
        args.func(args)
    except CommandException as ex:
        if ex.parser is not None:
            ex.parser.print_usage()
        print(ex.message)

if __name__ == "__main__":
    args = parser.parse_args()
    wib = WIB(args.wib_server)
    if args.command is None:
        import readline
        hist_file = os.path.expanduser('~/.wib_client_hist')
        if os.path.exists(hist_file):
            readline.read_history_file(hist_file)
        else:
            open(hist_file,'w').close()
        hist_start = readline.get_current_history_length()
        while True:
            try:
                line = input('[%s] >> '%args.wib_server).strip()
                handle_args(shlex.split(line))
            except KeyboardInterrupt:
                print()
                continue
            except EOFError:
                print()
                break
        hist_end = readline.get_current_history_length()
        readline.append_history_file(hist_end-hist_start,hist_file)
    else:
        handle_args([args.command]+args.args)
