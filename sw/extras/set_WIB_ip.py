# -*- coding: utf-8 -*-
"""
File Name: set_WIB_ip.py
Author: Eric Raguzin
Mail: eraguzin@bnl.gov
Description:
Created Time: 5/8/2022 4:50:34 PM
Last modified: 5/7/2022 3:30:42 PM
"""

#Based on https://docs.google.com/document/d/1gTWQ8o0j5eLqMWQ2_qdFCAeU2HQu0Fvyct5JglauJLI/edit

import sys
import os

reg_value = int(sys.argv[1],16)
#print(hex(reg_value))
#print(type(reg_value))
slot_addr = (reg_value & 0x7) + 1
crate_addr = ~((reg_value & 0xF0) >> 4) & 0xF
#print(hex(slot_addr))
#print(hex(crate_addr))
print (f"slot is {hex(slot_addr)} and crate is {hex(crate_addr)}")
def get_info(crate_addr, slot_addr):
    ip = "192.168.121.1"
    mac = "AA:BB:CC:DD:EE:10"
    name = None
    if (slot_addr == 0):
        pass
    elif (crate_addr == 0x0 or crate_addr == 0x6):
        ip = f"10.73.137.{(crate_addr*5)+19+slot_addr}"
        mac = f"AA:BB:CC:DD:0{crate_addr}:0{slot_addr}"
        name = f"np04-wib-{crate_addr}0{slot_addr}"
    elif (crate_addr == 0x5):
        if (slot_addr == 0x4):
            ip = "10.73.137.55"
            mac = "AA:BB:CC:DD:05:04"
            name = "np04-wib-504"
        elif (slot_addr == 0x5):
            ip = "10.73.137.49"
            mac = "AA:BB:CC:DD:05:05"
            name = "np04-wib-505"
        else:
            ip = f"10.73.137.{(crate_addr*5)+20+slot_addr}"
            mac = f"AA:BB:CC:DD:0{crate_addr}:0{slot_addr}"
            name = f"np04-wib-00{slot_addr}"
    elif (crate_addr == 0x8):
        ip = f"192.168.121.{19+slot_addr}"
        mac = f"AA:BB:CC:DD:08:0{slot_addr}"
    elif (crate_addr == 0x9):
        ip = f"10.73.138.{39+slot_addr}"
        mac = f"AA:BB:CC:DD:09:0{slot_addr}"
        name = f"np04-wib-{crate_addr}0{slot_addr}"
    elif (crate_addr == 0x0F):
        if (slot_addr > 1):
            ip = f"192.168.200.{slot_addr-1}"
    else:
        ip = f"10.73.137.{(crate_addr*5)+20+slot_addr}"
        mac = f"AA:BB:CC:DD:0{crate_addr}:0{slot_addr}"
        name = f"np04-wib-{crate_addr}0{slot_addr}" 

    return ip, mac, name

ip, mac, name = get_info(crate_addr, slot_addr)
print(f"IP address is {ip}, and MAC is {mac}, and name is {name}")
os.system(f"echo {ip}/24  > /etc/wib/ip")
os.system(f"echo {mac} > /etc/wib/mac")
os.system(f"poke 0xA00C0034 {crate_addr << 12}")
os.system(f"poke 0xA00C0034 {1 << 6}")
