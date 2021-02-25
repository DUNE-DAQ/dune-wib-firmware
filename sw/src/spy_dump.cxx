#include "unpack.h"
#include <cstdio>
#include <cmath>
#include <fstream>
#include <limits>

using namespace std;

void femb_seg(uint32_t *seg,char *femb) {
    uint16_t adc_vals[128];
    unpack14(seg,adc_vals);
    for (size_t i = 0; i < 40; i++) {
        size_t ch = u_to_ch[i];
        glog.log("%s U[%2ld] ADC[%2ld] CH[%2ld]: 0x%04X\n",femb,i,ch/16,ch%16,adc_vals[i]);
    }
    for (size_t i = 0; i < 40; i++) {
        size_t ch = v_to_ch[i];
        glog.log("%s V[%2ld] ADC[%2ld] CH[%2ld]: 0x%04X\n",femb,i,ch/16,ch%16,adc_vals[i+40]);
    }
    for (size_t i = 0; i < 48; i++) {
        size_t ch = x_to_ch[i];
        glog.log("%s X[%2ld] ADC[%2ld] CH[%2ld]: 0x%04X\n",femb,i,ch/16,ch%16,adc_vals[i+80]);
    }
}

void dump_frames(frame14 *buffer, size_t nframes) { 
    for (size_t i = 0; i < nframes; i++) {
        glog.log("start_frame:  0x%08X\n",buffer[i].start_frame);
        glog.log("wib_header_0: 0x%08X\n",buffer[i].wib_pre[0]);
        glog.log("wib_header_1: 0x%08X\n",buffer[i].wib_pre[1]);
        glog.log("wib_header_2: 0x%08X\n",buffer[i].wib_pre[2]);
        glog.log("wib_header_3: 0x%08X\n",buffer[i].wib_pre[3]);
        femb_seg(buffer[i].femb_a_seg,(char*)"FEMB_A");
        femb_seg(buffer[i].femb_b_seg,(char*)"FEMB_B");
        glog.log("wib_footer_0: 0x%08X\n",buffer[i].wib_post[0]);
        glog.log("wib_footer_1: 0x%08X\n",buffer[i].wib_post[1]);
        glog.log("idle_frame:   0x%08X\n",buffer[i].idle_frame);
    }
}

int main(int argc, char **argv) {
        
    ifstream bin_in(argv[1], ios::binary);
    if (!bin_in.is_open()) {
        glog.log("Could not open binary dump: %s\n",argv[1]);
        return 0;
    }
    bin_in.ignore( numeric_limits<streamsize>::max() );
    size_t length = bin_in.gcount();
    bin_in.clear();
    bin_in.seekg( 0, ios_base::beg );
    
    char *fbuf = (char*) malloc(length);
    bin_in.read(fbuf,length);
    bin_in.close();
    
    if (length != 0x200000) {
        glog.log("This does not look like a 2MB spy buffer file! Dumping raw data anyway...\n");
        dump_frames((frame14*)fbuf,length/sizeof(frame14));
    } else {
        glog.log("Dump spy buffer 0:\n");
        dump_frames((frame14*)fbuf,0x100000/sizeof(frame14));
        glog.log("Dump spy buffer 1:\n");
        dump_frames((frame14*)(fbuf+0x100000),0x100000/sizeof(frame14));
    }
    
    free(fbuf);
    return 0;
}
