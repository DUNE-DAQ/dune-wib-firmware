import zmq
import json
import numpy as np
import wib_pb2 as wibpb

class WIB:
    '''Encapsulates python methods for interacting with wib_server running on a WIB'''

    def __init__(self,wib_server='127.0.0.1'):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect('tcp://%s:1234'%wib_server)

    def send_command(self,req,rep):
        cmd = wibpb.Command()
        cmd.cmd.Pack(req)
        self.socket.send(cmd.SerializeToString())
        rep.ParseFromString(self.socket.recv())
        
    def defaults(self):
        req = wibpb.ConfigureWIB()
        #see wib.proto for meanings
        req.pulser = False
        req.cold = False
        req.adc_test_pattern = False
        for i in range(4):
            femb_conf = req.fembs.add();
            femb_conf.enabled = False
            femb_conf.test_cap = False
            femb_conf.gain = 2
            femb_conf.peak_time = 3
            femb_conf.baseline = 0
            femb_conf.pulse_dac = 0
            femb_conf.leak = 0
            femb_conf.leak_10x = False
            femb_conf.ac_couple = False
            femb_conf.buffer = 0
            femb_conf.strobe_skip = 255
            femb_conf.strobe_delay = 255
            femb_conf.strobe_length = 255
        return req
        
    def configure(self,config):
        
        if config is None:
            print('Loading defaults')
            req = self.defaults()
        else:
            print('Loading config')
            try:
                with open(config,'rb') as fin:
                    config = json.load(fin)
            except Exception as e:
                print('Failed to load config:',e)
                return
                
            req = wibpb.ConfigureWIB()
            req.cold = config['cold']
            req.pulser = config['pulser']
            if 'adc_test_pattern' in config:
                req.adc_test_pattern = config['adc_test_pattern']
            if 'frame_dd' in config:
                req.frame_dd = config['frame_dd']
            
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
        rep = wibpb.Status()
        self.send_command(req,rep);
        print(rep.extra.decode('ascii'))
        print('Successful: ',rep.success)
        return rep.success
        
    def acquire_data(self,buf0=True,buf1=True,deframe=True,channels=True,ignore_failure=False,trigger_command=0,trigger_rec_ticks=0,trigger_timeout_ms=0):
        print('Reading out WIB spy buffer')
        req = wibpb.ReadDaqSpy()
        req.buf0 = buf0
        req.buf1 = buf1
        req.deframe = deframe
        req.channels = channels
        req.trigger_command = trigger_command
        req.trigger_rec_ticks = trigger_rec_ticks
        req.trigger_timeout_ms = trigger_timeout_ms
        rep = wibpb.ReadDaqSpy.DeframedDaqSpy()
        self.send_command(req,rep)
        print('Successful:',rep.success)
        if not ignore_failure and not rep.success:
            return None
        num = rep.num_samples
        print('Acquired %i samples'%num)
        timestamps = np.frombuffer(rep.deframed_timestamps,dtype=np.uint64).reshape((2,num))
        samples = np.frombuffer(rep.deframed_samples,dtype=np.uint16).reshape((4,128,num))
        return timestamps,samples
    
    def print_timing_status(self,timing_status):
        print('--- PLL INFO ---')
        print('LOS:         0x%x'%(timing_status.los_val & 0x0f))
        print('OOF:         0x%x'%(timing_status.los_val >> 4))
        print('LOS FLG:     0x%x'%(timing_status.los_flg_val & 0x0f))
        print('OOF FLG:     0x%x'%(timing_status.los_flg_val >> 4))
        print('HOLD:        0x%x'%( (timing_status.los_val >> 5) & 0x1 ))
        print('LOL:         0x%x'%( (timing_status.los_val >> 1) & 0x1 ))
        print('HOLD FLG:    0x%x'%( (timing_status.lol_flg_val >> 5) & 0x1 ))
        print('LOL FLG:     0x%x'%( (timing_status.lol_flg_val >> 1) & 0x1 ))
        print('--- EPT INFO ---')
        print('EPT CDR LOS: 0x%x'%( (timing_status.ept_status >> 17) & 0x1 )) # bit 17 is CDR LOS as seen by endpoint
        print('EPT CDR LOL: 0x%x'%( (timing_status.ept_status >> 16) & 0x1 )) # bit 16 is CDR LOL as seen by endpoint
        print('EPT TS RDY:  0x%x'%( (timing_status.ept_status >> 8 ) & 0x1 )) # bit 8 is ts ready
        print('EPT STATE:   0x%x'%(  timing_status.ept_status & 0x0f )) # bits 3:0 are the endpoint state
