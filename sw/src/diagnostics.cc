#include "diagnostics.h"

#include <vector>

using namespace std;

template <typename T>
double mean(vector<T> vec) {
    double a = 0.0;
    for (size_t i = 0; i < vec.size(); i++) a += vec[i];
    return a / vec.size();
}

template <typename T>
double stdev(vector<T> vec) {
    double m = mean(vec);
    double a = 0.0;
    for (size_t i = 0; i < vec.size(); i++) a += pow(vec[i]-mean,2.0);
    return sqrt(a / vec.size());
}

bool adc_test_config(WIB_3ASIC &w, const bool femb_mask[4], bool cold) {
    wib::ConfigureWIB conf;
    conf.set_cold(cold);
    conf.set_pulser(false);
    conf.set_adc_test_pattern(true);
    for (size_t iFEMB = 0; iFEMB < 4; iFEMB++) {
        wib::ConfigureWIB::ConfigureFEMB *femb_conf = conf.add_fembs();
        femb_conf->set_enabled(femb_mask[iFEMB]);
        femb_conf->set_test_cap(false);
        femb_conf->set_gain(0);
        femb_conf->set_peak_time(0);
        femb_conf->set_baseline(0);
        femb_conf->set_pulse_dac(0);
        femb_conf->set_leak(0);
        femb_conf->set_leak_10x(false);
        femb_conf->set_ac_couple(false);
        femb_conf->set_buffer(1);
        femb_conf->set_strobe_skip(255);
        femb_conf->set_strobe_delay(255);
        femb_conf->set_strobe_length(255);
    }
    return w.configure_wib(conf);
}

bool acquire_data(WIB &w, const bool femb_mask[4], channel_data &dch0, channel_data &dch1) {
    char *buf0 = (femb_mask[0] || femb_mask[1]) ? new char[DAQ_SPY_SIZE] : NULL;
    char *buf1 = (femb_mask[2] || femb_mask[3]) ? new char[DAQ_SPY_SIZE] : NULL;
    bool success = w.read_daq_spy(buf0,buf1);
    
    const size_t nframes = DAQ_SPY_SIZE/sizeof(frame14);
    const size_t ch_len = nframes*sizeof(uint16_t);

    if (buf0) deframe_data((frame14*)buf0,nframes,dch0);
    if (buf1) deframe_data((frame14*)buf1,nframes,dch1);
    
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
        glog.log("FEMB %i ADC test pattern validation: %s\n",iFEMB,valid?"passed":"failed");
        all_valid &= valid;
    }
    return all_valid;
}
