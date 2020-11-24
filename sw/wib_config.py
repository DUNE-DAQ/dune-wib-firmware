#!/usr/bin/env python3

import os
import sys
import time
import argparse
import zmq
import json

import wib_pb2 as wib


class WIB:
    def __init__(self,wib_server='127.0.0.1'):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect('tcp://%s:1234'%wib_server)

    def send_command(self,req,rep):
        cmd = wib.Command()
        cmd.cmd.Pack(req)
        self.socket.send(cmd.SerializeToString())
        rep.ParseFromString(self.socket.recv())
        
    def configure_wib(self,config):
        print('Loading config')
        try:
            with open(config,'rb') as fin:
                config = json.load(fin)
        except Exception as e:
            print('Failed to load config:',e)
            return
            
        print('Configuring FEMBs')
        req = wib.ConfigureWIB()
        req.cold = config['cold']
        for i in range(4):
            femb_conf = req.fembs.add();
            
            femb_conf.enabled = config['enabled_fembs'][i]
            
            fconfig = config['femb_configs'][i]
            
            #see wib.proto for meanings
            femb_conf.test_cap = fconfig['test_cap']
            femb_conf.gain = fconfig['gain']
            femb_conf.peak_time = fconfig['peak_time']
            femb_conf.baseline = fconfig['baseline']
            femb_conf.pulse_dac = fconfig['pulse_dac']

            femb_conf.leak = fconfig['leak']
            femb_conf.leak_10x = fconfig['leak_10x']
            femb_conf.ac_couple = fconfig['ac_couple']
            femb_conf.buffer = fconfig['buffer']

            femb_conf.strobe_skip = fconfig['strobe_skip']
            femb_conf.strobe_delay = fconfig['strobe_delay']
            femb_conf.strobe_length = fconfig['strobe_length']
        
        print('Sending ConfigureWIB command')
        rep = wib.Status()
        self.send_command(req,rep);

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visually display data from a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--config','-C',default='default.json',help='WIB configuration to load [default.json]')
    args = parser.parse_args()
    
    w = WIB(wib_server=args.wib_server)
    w.configure_wib(args.config)
    
