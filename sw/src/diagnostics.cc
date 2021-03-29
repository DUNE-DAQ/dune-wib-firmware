#include "diagnostics.h"

#include <vector>

using namespace std;

bool adc_test_config(WIB_3ASIC &w, const bool femb_mask[4], bool cold, bool test = true) {
    wib::ConfigureWIB conf;
    conf.set_cold(cold);
    conf.set_pulser(false);
    conf.set_adc_test_pattern(test);
    for (size_t iFEMB = 0; iFEMB < 4; iFEMB++) {
        wib::ConfigureWIB::ConfigureFEMB *femb_conf = conf.add_fembs();
        femb_conf->set_enabled(femb_mask[iFEMB]);
        // defaults for iceberg/protodune p1 chips
        femb_conf->set_test_cap(false);
        femb_conf->set_gain(2);
        femb_conf->set_peak_time(3);
        femb_conf->set_baseline(0);
        femb_conf->set_pulse_dac(0);
        femb_conf->set_leak(0);
        femb_conf->set_leak_10x(false);
        femb_conf->set_ac_couple(false);
        femb_conf->set_buffer(0);
        femb_conf->set_strobe_skip(255);
        femb_conf->set_strobe_delay(255);
        femb_conf->set_strobe_length(255);
    }
    return w.configure_wib(conf);
}

bool acquire_data(WIB &w, const bool femb_mask[4], channel_data &dch0, channel_data &dch1) {
    char *buf0 = (femb_mask[0] || femb_mask[1]) ? new char[DAQ_SPY_SIZE] : NULL;
    char *buf1 = (femb_mask[2] || femb_mask[3]) ? new char[DAQ_SPY_SIZE] : NULL;
    int nframes0, nframes1;
    bool success = w.read_daq_spy(buf0,&nframes0,buf1,&nframes1);

    if (buf0) deframe_data((frame14*)buf0,nframes0,dch0);
    if (buf1) deframe_data((frame14*)buf1,nframes1,dch1);
    
    delete [] buf0;
    delete [] buf1;
    
    return success;
}

bool check_test_pattern(WIB_3ASIC &w, const bool femb_mask[4], bool cold) {
    glog.log("Configuring ADC test pattern on enabled FEMBs\n");
    bool conf_res = adc_test_config(w,femb_mask,cold);
    if (!conf_res) {
        glog.log("Failed to configure WIB with ADC test pattern enabled\n");
        return false;
    }
    channel_data fembs01;
    channel_data fembs23;
    glog.log("Acquiring ADC test pattern on enabled FEMBs\n");
    bool acq_res = acquire_data(w,femb_mask,fembs01,fembs23);
    if (!acq_res) {
        glog.log("Failed to acquire data from spy buffers\n");
        return false;
    }
    bool all_valid = true;
    for (size_t iFEMB = 0; iFEMB < 4; iFEMB++) {
        if (!femb_mask[iFEMB]) continue;
        channel_data &dat = (iFEMB < 2 ? fembs01 : fembs23);
        int idat = iFEMB%2;
        bool valid = true;
        for (size_t ich = 0; ich < 128; ich++) {
            double m = mean(dat.channels[idat][ich]);
            if (ich % 8 == 0) {
                valid &= m > 14000;
            } else if (ich % 8 == 2) {
                valid &= m < 4000;
            } else {
                valid &= (m > 6000) && (m < 10000);
            }
        }
        #ifdef SIMULATION
        valid = true;
        #endif
        glog.log("FEMB %i ADC test pattern validation: %s\n",iFEMB,valid?"passed":"failed");
        all_valid &= valid;
    }
    adc_test_config(w,femb_mask,cold,false);
    return all_valid;
}
