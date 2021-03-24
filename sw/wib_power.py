#!/usr/bin/env python3

import os
import sys
import time
import argparse

from wib import WIB
import wib_pb2 as wibpb

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Change the FEMB power state on a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--cold','-c',action='store_true',help='The FEMBs will load the cold configuration with this option [default: warm]')
    parser.add_argument('--stage','-s',choices=['full','pre','post'],default='full',help='Run full power ON sequence or pre/post ADC synchronization stages [default: full]')
    parser.add_argument('FEMB_0',choices=['on','off'],help='Power FEMB_0')
    parser.add_argument('FEMB_1',choices=['on','off'],help='Power FEMB_1')
    parser.add_argument('FEMB_2',choices=['on','off'],help='Power FEMB_2')
    parser.add_argument('FEMB_3',choices=['on','off'],help='Power FEMB_3')
    args = parser.parse_args()
    
    wib = WIB(args.wib_server)
    req = wibpb.PowerWIB()
    req.femb0 = args.FEMB_0 == 'on'
    req.femb1 = args.FEMB_1 == 'on'
    req.femb2 = args.FEMB_2 == 'on'
    req.femb3 = args.FEMB_3 == 'on'
    req.cold = args.cold
    if args.stage == 'full':
        req.stage = 0
    elif args.stage == 'pre':
        req.stage = 1
    elif args.stage == 'post':
        req.stage = 2
    rep = wibpb.Status()
    print('Sending PowerWIB command...')
    wib.send_command(req,rep)
    print(rep.extra.decode('ascii'))
    print('Successful:',rep.success)
    
    
