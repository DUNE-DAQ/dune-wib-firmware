#!/usr/bin/env python3

import os
import sys
import time
import argparse

from wib import WIB
import wib_pb2 as wibpb

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Send a configuration json document to a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--config','-C',default=None,help='WIB configuration to load [defaults]')
    args = parser.parse_args()
    
    wib = WIB(args.wib_server)
    wib.configure(args.config)
