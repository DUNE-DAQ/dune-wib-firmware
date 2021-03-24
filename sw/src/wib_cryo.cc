#include "wib_cryo.h"

WIB_CRYO::WIB_CRYO() : WIB() {
    for (int i = 0; i < 4; i++) {
        this->femb[i] = new FEMB_CRYO(i);
    }
}

WIB_CRYO::~WIB_CRYO() {
    for (int i = 0; i < 4; i++) {
        delete this->femb[i];
    }
}

bool WIB_CRYO::set_pulser(bool on) {
    
    //FIXME do CRYO pulser control
    
    return false;
}

bool WIB_CRYO::reset_frontend() {
    bool success = true;
    glog.log("Disabling front end power\n");
    femb_power_en_ctrl(0, 0x00);
    femb_power_en_ctrl(1, 0x00);
    femb_power_en_ctrl(2, 0x00);
    femb_power_en_ctrl(3, 0x00);
    femb_power_config();
    
    // FIXME add logic to reset WIB_CRYO frontend
    
    return true;
}

bool WIB_CRYO::power_wib(const wib::PowerWIB &conf) {

    // FIXME do CRYO power on sequence
    
    return false;
}

bool WIB_CRYO::configure_wib(const wib::ConfigureWIB &conf) {

    if (conf.fembs_size() != 4) {
        glog.log("Must supply exactly 4 FEMB configurations\n");
        return false;
    }
    
    bool fembs_powered = true;
    for (int i = 0; i < 4; i++) { // Check FEMB power state (enabled FEMBs must be ON)
        if (conf.fembs(i).enabled()) {
            fembs_powered &= frontend_power[i];
        }
    }
    if (!fembs_powered) {
        glog.log("Enabled FEMBs must be powered\n");
        return false;
    }
    
    glog.log("Reconfiguring WIB\n"); 
    
    // FIXME do CRYO config
    
    return false;
}
