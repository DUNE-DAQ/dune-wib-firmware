#!/usr/bin/env python3

import os
import sys
import time
import argparse

from wib import WIB
import wib_pb2 as wibpb

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Change the FEMB voltages on a WIB (first turns FEMBs OFF)')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--dc2dc-o1','--o1',default=4.0,type=float,help='DC2DC O1 output voltage [4.0V]')
    parser.add_argument('--dc2dc-o2','--o2',default=4.0,type=float,help='DC2DC O2 output voltage [4.0V]')
    parser.add_argument('--dc2dc-o3','--o3',default=4.0,type=float,help='DC2DC O3 output voltage [4.0V]')
    parser.add_argument('--dc2dc-o4','--o4',default=4.0,type=float,help='DC2DC O4 output voltage [4.0V]')
    parser.add_argument('--ldo-a0','--a0',default=2.5,type=float,help='LDO A0 output voltage [2.5V]')
    parser.add_argument('--ldo-a1','--a1',default=2.5,type=float,help='LDO A1 output voltage [2.5V]')
    args = parser.parse_args()
    
    wib = WIB(args.wib_server)
    req = wibpb.ConfigurePower()
    
    req.dc2dc_o1 = args.dc2dc_o1;
    req.dc2dc_o2 = args.dc2dc_o2;
    req.dc2dc_o3 = args.dc2dc_o3;
    req.dc2dc_o4 = args.dc2dc_o4;
    req.ldo_a0 = args.ldo_a0;
    req.ldo_a1 = args.ldo_a1;
    
    rep = wibpb.Status()
    print('Sending ConfigurePower command...')
    wib.send_command(req,rep)
    print(rep.extra.decode('ascii'))
    print('Successful:',rep.success)
    
    
