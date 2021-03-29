#!/usr/bin/env python3

import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig
import matplotlib
import h5py

matplotlib.rcParams['figure.figsize'] = [10,7]
matplotlib.rcParams['xtick.top'] = True
matplotlib.rcParams ['xtick.direction'] = 'in'
matplotlib.rcParams['xtick.minor.visible'] = True
matplotlib.rcParams['ytick.right'] = True
matplotlib.rcParams['ytick.direction'] = 'in'
matplotlib.rcParams['ytick.minor.visible'] = True
matplotlib.rcParams['font.size'] = 19
matplotlib.rcParams['font.family']= 'DejaVu Serif'
matplotlib.rcParams['mathtext.default'] = 'regular'
matplotlib.rcParams['errorbar.capsize'] = 3
matplotlib.rcParams['figure.facecolor'] = (1,1,1)

from wib import WIB
import wib_pb2 as wibpb

def configure_pulser_run(wib,pulser_dac,femb_mask=[False,False,False,False],cold=False):
    req = wib.defaults()
    req.cold = cold
    req.pulser = True
    for i in range(4):
        femb_conf = req.fembs[i]
        femb_conf.enabled = femb_mask[i]
        if not femb_conf.enabled:
            continue
        #see wib.proto for meanings
        femb_conf.test_cap = True
        femb_conf.pulse_dac = pulser_dac
        femb_conf.strobe_skip = 255
        femb_conf.strobe_delay = 255
        femb_conf.strobe_length = 255

    rep = wibpb.Status()
    print('Configuring WIB with FEMB mask %s for pulser run with DAC value %i'%(str(femb_mask),pulser_dac))
    wib.send_command(req,rep)
    if not rep.success:
        print(rep.extra.decode('ascii'))
    print('Successful:',rep.success)
    return rep.success

def take_data(wib,fnames,pulser_dacs=[0,5,10,15,20],num_acquisitions=20,cold=False,ignore_failure=False):
    try:
        hfs = None
        femb_mask = [fnames[idx].lower() != 'none' if idx < len(fnames) else False for idx in range(4)]
        hfs = [(idx,h5py.File(fname,'w')) for idx,fname in enumerate(fnames) if femb_mask[idx]]
        for pulser_dac in pulser_dacs:
            grps = [(idx,hf.create_group('dac%i'%pulser_dac)) for idx,hf in hfs]
            success = configure_pulser_run(wib,pulser_dac,femb_mask=femb_mask,cold=cold) 
            if not success and not ignore_failure:
                raise Exception('Failed to configure FEMB. See WIB log for more info.')
            for i in range(num_acquisitions):
                buf0 = femb_mask[0] or femb_mask[1]
                buf1 = femb_mask[2] or femb_mask[3]
                data = wib.acquire_data(buf0=buf0,buf1=buf1,ignore_failure=ignore_failure)
                if data is None:
                    raise Exception('Failed to acquire data from WIB. See WIB log for more info.')
                timestamps,samples = data
                for idx,gr in grps:
                    gr.create_dataset('ev%i'%i,data=samples[idx],compression='gzip',compression_opts=9)
    except:
        raise
    finally:
        if hfs is not None:
            for idx,hf in hfs:
                hf.close()
        
def analyze_ch(ch,ped_start=-100,ped_end=-15,prominence=100):
    peaks,*_ = sig.find_peaks(ch,prominence=prominence)
    heights = []
    for peak in peaks:
        if peak < -ped_start:
            continue
        baseline = np.mean(ch[peak+ped_start:peak+ped_end])
        heights.append(ch[peak]-baseline)
    return heights

def analyze_data(fname):
    pulser_dacs = []
    ch_mean_for_dacs = []
    ch_rms_for_dacs = []
    with h5py.File(fname,'r') as hf:
        pulser_settings = np.sort([int(key[3:]) for key in hf.keys()])
        for pulser_dac in pulser_settings:
            print('Analyzing DAC value %i' %pulser_dac)
            gr = hf['dac%i'%pulser_dac]
            ch_heights = [[] for ch in range(128)]
            nev = len(gr.keys())
            for ev in range(nev):
                femb_samples = gr['ev%i'%ev]
                for i,ch in enumerate(femb_samples):
                    #require the max-min of a bipolar pulse to be at least max(50,100*pulser_dac)
                    heights = analyze_ch(ch,prominence=max(50,100*pulser_dac))
                    ch_heights[i].extend(heights)
            pulser_dacs.append(pulser_dac)
            ch_mean_for_dacs.append([np.mean(x) for x in ch_heights])
            ch_rms_for_dacs.append([np.std(x) for x in ch_heights])
    pulser_dacs = np.asarray(pulser_dacs)
    ch_mean_for_dacs = np.asarray(ch_mean_for_dacs).T
    ch_rms_for_dacs = np.asarray(ch_rms_for_dacs).T
    return pulser_dacs,ch_mean_for_dacs,ch_rms_for_dacs

linestyle_list = ['-','--','-.',':']
color_list = ['tab:blue','tab:orange','tab:green','tab:red']
def create_plots(prefix,pulser_dacs,ch_mean_for_dacs,ch_rms_for_dacs):
    print('Generating plots in %s'%prefix)
    for i,(mean,rms) in enumerate(zip(ch_mean_for_dacs,ch_rms_for_dacs)):
        ch = i%16
        c = color_list[ch%4]
        ls = linestyle_list[ch//4]
        plt.errorbar(pulser_dacs,mean,yerr=rms,label='Ch %i'%i,c=c,ls=ls)
        plt.xlabel('Pulser DAC (ADC)')
        plt.ylabel('Pulse Height (ADC)')
        if (i+1)%16 == 0:
            plt.legend(fontsize='xx-small')
            cd_idx = ((i+1)//16-1)
            plt.title('COLDADC %i'%cd_idx)
            plt.savefig(os.path.join(prefix,'COLDADC_%i.pdf'%cd_idx),bbox_inches='tight')
            plt.close()
    
def acquire(args):
    take_data(WIB(args.wib_server),args.femb_data,num_acquisitions=args.nacq,cold=args.cold,ignore_failure=args.ignore_failure)
    
def analyze(args):
    pulser_dacs,ch_mean_for_dacs,ch_rms_for_dacs = analyze_data(args.femb_data)
    if not os.path.exists(args.plot_loc):
        os.mkdir(args.plot_loc)
    create_plots(args.plot_loc,pulser_dacs,ch_mean_for_dacs,ch_rms_for_dacs)
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Acquire pulser data from FEMBs and/or perform linearity test',add_help=False)
    parser.add_argument('--help','-h',nargs='?',const=True,help='show help message and exit',metavar='subcommand')
    sub = parser.add_subparsers(title='subcommands',help='subcommand help',dest='cmd')
    
    acquire_parser = sub.add_parser('acquire',help='Acquire data from a WIB using the spy buffer and save to HDF5 file')
    acquire_parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    acquire_parser.add_argument('--ignore_failure','-i',action='store_true',help='Ignore failure in configuration and keep taking data')
    acquire_parser.add_argument('--cold','-c',default=False,action='store_true',help='The FEMBs will load the cold configuration with this option [default: warm]')
    acquire_parser.add_argument('--nacq','-n',default=20,type=int,help='Number of acquisitions per pulser DAC setting [20]')
    acquire_parser.add_argument('femb_data',nargs='+',help='Name for HDF5 file for saving FEMB pulser data (one per FEMB to configure, or ''none'' to skip a FEMB)')
    
    analyze_parser = sub.add_parser('analyze',help='Analyze HDF5 file to find pulse peak values and produce linaerity plots')
    analyze_parser.add_argument('femb_data',help='Name for HDF5 file containing FEMB pulser data')
    analyze_parser.add_argument('plot_loc',help='Name of directory to save ADC linearity plots')
    
    args = parser.parse_args()
    if args.help:
        if args.help == 'acquire':
            acquire_parser.print_help()
        elif args.help == 'analyze':
            analyze_parser.print_help()
        else:
            parser.print_help()
    elif args.cmd == 'acquire':
        acquire(args)
    elif args.cmd == 'analyze':
        analyze(args)
    else:
        parser.print_usage()
    
