#include "wib_3asic.h"
#include "femb_3asic.h"
#include "diagnostics.h"

#include <unistd.h>

using namespace std;

WIB_3ASIC::WIB_3ASIC() : WIB() {
    for (int i = 0; i < 4; i++) {
        this->femb[i] = new FEMB_3ASIC(i);
    }
}

WIB_3ASIC::~WIB_3ASIC() {
    for (int i = 0; i < 4; i++) {
        delete this->femb[i];
    }
}

bool WIB_3ASIC::script_extra(const vector<string> &tokens) {
    string cmd(tokens[0]);
    if (cmd == "fast") {
        if (tokens.size() != 2) {
            glog.log("Invalid arguments to fast\n");
            return false;
        }
        string fast(tokens[1]);
        if (fast == "reset") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_RESET);
        } else if (fast == "act") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_ACT);
        } else if (fast == "sync") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_SYNC);
        } else if (fast == "edge") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_EDGE);
        } else if (fast == "idle") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_IDLE);
        } else if (fast == "edge_act") {
            FEMB_3ASIC::fast_cmd(FAST_CMD_EDGE_ACT);
        } else {
            glog.log("Unknown fast command: %s\n",fast.c_str());
            return false;
        }
        return true;
    } else if (cmd == "cd-i2c") {
         // cd-i2c femb coldata chip page addr data
        uint8_t femb_idx = (uint8_t)strtoull(tokens[1].c_str(),NULL,10);
        uint8_t coldata_idx = (uint8_t)strtoull(tokens[2].c_str(),NULL,10);
        uint8_t chip_addr = (uint8_t)strtoull(tokens[3].c_str(),NULL,16);
        uint8_t reg_page = (uint8_t)strtoull(tokens[4].c_str(),NULL,16);
        uint8_t reg_addr = (uint8_t)strtoull(tokens[5].c_str(),NULL,16);
        uint8_t data = (uint8_t)strtoull(tokens[6].c_str(),NULL,16);
        
        cdpoke(femb_idx, coldata_idx, chip_addr, reg_page, reg_addr, data);
        return true;
    } else {
        glog.log("Invalid script command: %s\n", tokens[0].c_str());
    }
    return false;
}

bool WIB_3ASIC::reset_frontend() {
    bool success = true;
    glog.log("Disabling front end power\n");
    femb_power_set(0,false);
    femb_power_set(1,false);
    femb_power_set(2,false);
    femb_power_set(3,false);
    femb_power_config();
    if (!pll_initialized) {
        success &= reset_timing_endpoint();
    }
    femb_rx_mask(0xFFFF); //all disabled
    glog.log("Resetting FEMB receiver\n");
    femb_rx_reset();
    if (!felix_initialized) {
        glog.log("Resetting FELIX transmitter\n");
        felix_tx_reset();
    }
    frontend_initialized = true;
    return success;
}

bool WIB_3ASIC::femb_power_set(int femb_idx, bool on, bool cold) {
    bool power_res = true;
    if (on) {
        if (frontend_power[femb_idx]) {
            glog.log("FEMB %i already on\n",femb_idx);
            return power_res;
        }
        glog.log("Powering on FEMB %i COLDATA\n",femb_idx);
        femb_power_en_ctrl(femb_idx, 0x6B); //COLDATA ON
        usleep(1000000);
        glog.log("Loading %s COLDATA config\n",cold?"COLD":"WARM");
        power_res &= femb[femb_idx]->configure_coldata(cold,FRAME_14,1); //default config
        if (!power_res) {
            glog.log("Failed to configure COLDATA; aborting power on\n");
            femb_power_set(femb_idx,false);
            return false;
        }
        glog.log("Powering on FEMB %i COLDADC\n",femb_idx);
        femb_power_en_ctrl(femb_idx, 0xFF); //COLDATA+COLDADC ON
        usleep(1000000);
        glog.log("Loading %s COLDADC config\n",cold?"COLD":"WARM");
        power_res &= femb[femb_idx]->configure_coldadc(cold); //default config
        if (!power_res) {
            glog.log("Failed to configure COLDADC; aborting power on\n");
            femb_power_set(femb_idx,false);
            return false;
        }
        if (power_res) glog.log("FEMB %i powered succesfully\n",femb_idx);
        //does not turn on VDDA2P5 or VDDD2P5
    } else {
        glog.log("Powering off FEMB %i\n",femb_idx);
        femb_power_en_ctrl(femb_idx, 0x00); //CLDATA+COLDADC OFF
    }
    return power_res;
}

void WIB_3ASIC::enable_stamp_sync(bool on) {
    uint32_t value = io_reg_read(&this->regs, REG_FAKE_TIME_CTRL);
    if (on) {
      value |= (1<<2);
    } else {
      value &= ~(1<<2);
    }
    io_reg_write(&this->regs, REG_FAKE_TIME_CTRL, value);
}

bool WIB_3ASIC::set_alignment(uint32_t cmd_stamp_sync) {
    uint32_t prev = io_reg_read(&this->regs, REG_FAKE_TIME_CTRL);
    uint32_t mask = 0xffffffff ^ (0x7fff << 16);
    uint32_t write = (cmd_stamp_sync & 0x7fff) << 16;
    io_reg_write(&this->regs, REG_FAKE_TIME_CTRL, (prev & mask) | write);
    return true;
}

bool WIB_3ASIC::set_edge_delay(uint8_t edge_delay) {
    if (edge_delay > 32) {
        glog.log("Edge delay of %d is too large, aborting setting", edge_delay);
	return false;
    }
    uint32_t prev = io_reg_read(&this->regs, 0x0020/4);
    uint32_t mask = 0xffffffff ^ (0x3F << 9);
    uint32_t write = (edge_delay & 0x3F) << 9;
    io_reg_write(&this->regs, 0x0020/4, (prev & mask) | write);
    return true;
}

bool WIB_3ASIC::set_channel_map(int detector_type) {
  uint32_t value = io_reg_read(&this->regs, REG_FW_CTRL);
  if (detector_type == 1 || detector_type == 2) {
    value &= ~(1<<21);
  } else if (detector_type == 3) {
    value |= (1<<21);
  } else {
    glog.log("Unknown detector type %i, leaving channel map unmodified", detector_type);
    return false;
  }
  io_reg_write(&this->regs, REG_FW_CTRL, value);
  return true;
}

bool WIB_3ASIC::reset_crc_bits() {
    uint32_t value = io_reg_read(&this->regs, REG_FW_CTRL);
    value |= (1<<20);
    io_reg_write(&this->regs, REG_FW_CTRL, value);
    usleep(500000);
    value &= ~(1<<20);
    io_reg_write(&this->regs, REG_FW_CTRL, value);
    return true;
}

bool WIB_3ASIC::check_alignment_delay(int fembIdx) {
  uint32_t value;
  switch (fembIdx) {
  case 0:
    value = io_reg_read(&this->regs, REG_COLDATA_ALIGNMENT_0);
    break;
  case 1:
    value = io_reg_read(&this->regs, REG_COLDATA_ALIGNMENT_1);
    break;
  case 2:
    value = io_reg_read(&this->regs, REG_COLDATA_ALIGNMENT_2);
    break;
  case 3:
    value = io_reg_read(&this->regs, REG_COLDATA_ALIGNMENT_3);
    break;
  default:
    glog.log("FEMB %i does not exist, cannot check alignment delay\n", fembIdx);
    return false;
  }

  for (int link = 0; link < 4; link++) {
    uint32_t mask = 0xFF << (8*link);
    uint32_t delayValue = (value & mask) >> (8*link);
    if (delayValue > 0x7F) {
      glog.log("FEMB %i link %i has bad alignment delay of %02X\n", fembIdx, link, delayValue);
      return false;
    }
  }
  return true;
}

bool WIB_3ASIC::femb_rx_mask(uint32_t value, uint32_t mask) {
    uint32_t prev = io_reg_read(&this->regs,REG_LINK_MASK);
    value = (prev & (~mask)) | (value & mask);
    io_reg_write(&this->regs,REG_LINK_MASK,value);
    return true;
}

bool WIB_3ASIC::femb_rx_reset() {
    //rx_reset is bit 13
    uint32_t value = io_reg_read(&this->regs,REG_FW_CTRL);
    value |= (1<<13);
    io_reg_write(&this->regs,REG_FW_CTRL,value);
    value &= ~(1<<13);
    io_reg_write(&this->regs,REG_FW_CTRL,value);
    return true;
}

uint8_t WIB_3ASIC::cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    return this->femb[femb_idx]->i2c_read(coldata_idx,chip_addr,reg_page,reg_addr);
}

void WIB_3ASIC::cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data) {
    this->femb[femb_idx]->i2c_write(coldata_idx,chip_addr,reg_page,reg_addr,data);
}

bool WIB_3ASIC::set_pulser(bool on) {
    if (on != pulser_on) {
        glog.log(on ? "Starting pulser\n" : "Stopping pulser\n");
        bool pulser_res = true;
        for (int i = 0; i < 4; i++) { 
            if (frontend_power[i]) {
                pulser_res &= femb[i]->set_fast_act(ACT_LARASIC_PULSE);
            }
        }
        FEMB_3ASIC::fast_cmd(FAST_CMD_ACT); // Perform ACT
        pulser_on = on;
        if (!pulser_res) glog.log("ERROR: Pulser failed to toggle, pulser state unknown\n");
        return pulser_res;
    } else {
        glog.log(on ? "Pulser already started\n" : "Pulser already stopped\n");
        return true;
    }
}

// convenience method to index config
bool femb_i_on(const wib::PowerWIB &conf, int i) {
    switch (i) {
        case 0:
            return conf.femb0();
        case 1:
            return conf.femb1();
        case 2:
            return conf.femb2();
        case 3:
            return conf.femb3();
        default:
            return false;
    }
}

bool WIB_3ASIC::power_wib(const wib::PowerWIB &conf) {

    if (!frontend_initialized) {
        glog.log("Initializing front end...\n");
        if (!reset_frontend()) {
            glog.log("Failed to start frontend electronics\n");
            return false;
        }
    }
    
    //pulser will be off for any new FEMBs, so turn off for all old fembs
    bool pulser_res = set_pulser(false);
    
    bool power_res = true;
    bool any_on = false;
    for (int i = 0; i < 4; i++) {
        if (conf.stage() == 2) { // only check that FEMBs were previously powered on
            bool femb_state = frontend_power[i] == femb_i_on(conf,i);
            if (!femb_state) {
                glog.log("FEMB %i is %s but requested to be %s\n",i,frontend_power[i]?"ON":"OFF",femb_i_on(conf,i)?"ON":"OFF");
            }
            power_res &= femb_state;
            if (femb_i_on(conf,i)) {
                any_on = true;
            }
        } else { // other stages turn on FEMB power regulators
            power_res &= femb_power_set(i, femb_i_on(conf, i)); // Sequences COLDATA -> COLDADC (except VDDA2P5, VDDD2P5)
            if (femb_i_on(conf,i)) {
                any_on = true;
                femb[i]->set_fast_act(ACT_RESET_COLDADC); // Prepare COLDADC reset
            }
        }
    }
    
    if (!power_res) { // Break out here if any FEMBs failed to power ON
        glog.log("Failed to power on enabled FEMBs, aborting\n");
        return false;
    }
    
    if (!any_on) return true; // Break out here of no FEMBs are ON
    
    if (conf.stage() == 0) { // run the full sequence including a WIB-local ADC sync
        glog.log("Resetting and synchronizing COLDADCs\n");
        FEMB_3ASIC::fast_cmd(FAST_CMD_EDGE_ACT); // Perform EDGE+ACT    
    } else if (conf.stage() == 1) { // stop here and way for some other utility to perform the EDGE+ACT globally
        glog.log("Waiting for external COLDADC reset and synchronization\n");
        // pdtbutler mst PROD_MASTER send-cmd 0 11
        io_reg_write(&this->regs,REG_TIMING_CMD_1,0x0b00,0xFF00); // timing command 0xb == 11 will run EDGE+ACT
        for (int i = 0; i < 4; i++) {
            if (femb_i_on(conf,i)) {
                // disable VDDA VDDD power in prep for external reset & sync
                power_res &= femb[i]->set_control_reg(2,false,false); //VDDA on U1 ctrl_0/ctrl_1
                power_res &= femb[i]->set_control_reg(3,false,false);  //VDDD L/R on U2 ctrl_0/ctrl_1
            }
        }
        return power_res;
    } else if (conf.stage() == 2) { // resume power on sequence after a global EDGE+ACT
        glog.log("Resuming power ON after external COLDADC reset and synchronization\n");
        io_reg_write(&this->regs,REG_TIMING_CMD_1,0x00000,0xFF00); // zero out timing commands
    }
    
    for (int i = 0; i < 4; i++) {
        if (femb_i_on(conf,i)) {
            //Additional steps to turn on analog chips via COLDATA control regs
            glog.log("Loading %s COLDADC config for FEMB %i\n",conf.cold()?"COLD":"WARM",i);
            power_res &= femb[i]->configure_coldadc(conf.cold()); //default config
            glog.log("Enabling FEMB %i U1 control signals\n",i);
            power_res &= femb[i]->set_control_reg(2,true,true); //VDDA on U1 ctrl_1/ctrl_0
            usleep(100000);
            glog.log("Enabling FEMB %i U2 control_0 signal\n",i);
            power_res &= femb[i]->set_control_reg(3,false,true);  //VDDD L on U2 ctrl_0
            usleep(100000);
            glog.log("Enabling FEMB %i U2 control_1 signal\n",i);
            power_res &= femb[i]->set_control_reg(3,true,true);  //VDDD R on U2 ctrl_1
            usleep(100000);
            if (!power_res) {
                glog.log("Failed to enable COLDADC power for FEMB %i, aborting\n",i);
                return false;
            }
        }
    }
    
    glog.log("Running power-on diagnostics\n");
    bool adc_test_res = check_test_pattern(*this,frontend_power,conf.cold());
    //glog.log("Eric is skipping the test pattern check");
    //adc_test_res = true;

    bool good_alignment = true;
    for (int i = 0; i < 4; i++) {
        if (femb_i_on(conf,i)) {
	  good_alignment &= check_alignment_delay(i);
	}
    }
    
    return pulser_res && power_res && adc_test_res && good_alignment;
}

bool WIB_3ASIC::configure_wib(const wib::ConfigureWIB &conf) {

    if (conf.fembs_size() != 4) {
        glog.log("Must supply exactly 4 FEMB configurations\n");
        return false;
    }
    
    if (!is_endpoint_locked()) {
        glog.log("Timing endpoint in bad state\n");
	return false;
        #ifndef SIMULATION
        //return false;
        #endif
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

    enable_stamp_sync(false);

    int detector_type = conf.detector_type();
    if (detector_type == 0) {
      detector_type = getDetectorType();
      glog.log("Obtained detector type %i from crate ID\n", detector_type);
    }

    // Write cable delay value for timestamp synchronization
    if (detector_type == 1) {
      set_alignment(0x7fea);
      set_edge_delay(6);
    } else if (detector_type == 2) {
      set_alignment(0x7fe6);
      set_edge_delay(2);
    } else if (detector_type == 3) {
      set_alignment(0x7fe4);
      set_edge_delay(0);
    }

    // Set appropriate channel map
    set_channel_map(detector_type);
    if (conf.adc_test_pattern()) {
      set_channel_map(1);
    }
    
    bool coldata_res = true;
    for (int i = 0; i < 4; i++) { // Configure COLDATA
        if (conf.fembs(i).enabled()) coldata_res &= femb[i]->configure_coldata(conf.cold(),conf.frame_dd()?FRAME_DD:FRAME_14,detector_type);
    }
    if (coldata_res) {
        glog.log("COLDATA configured\n");
    } else {
        glog.log("COLDATA configuration failed!\n");
    }
    
    coldadc_conf *adc_conf = NULL;
    if (conf.has_adc_conf()) {
        glog.log("Using provided set of alternate COLDADC values\n");
        const wib::ConfigureWIB::ConfigureCOLDADC &adc_conf_msg = conf.adc_conf();
        adc_conf = new coldadc_conf;
        adc_conf->reg_0 = adc_conf_msg.reg_0();
        adc_conf->reg_4 = adc_conf_msg.reg_4();
        adc_conf->reg_24 = adc_conf_msg.reg_24();
        adc_conf->reg_25 = adc_conf_msg.reg_25();
        adc_conf->reg_26 = adc_conf_msg.reg_26();
        adc_conf->reg_27 = adc_conf_msg.reg_27();
        adc_conf->reg_29 = adc_conf_msg.reg_29();
        adc_conf->reg_30 = adc_conf_msg.reg_30();
    }
    
    bool coldadc_res = true;
    for (int i = 0; i < 4; i++) { // Configure COLDADCs
         if (conf.fembs(i).enabled()) coldadc_res &= femb[i]->configure_coldadc(conf.cold(),conf.adc_test_pattern(),adc_conf,conf.fembs(i).buffer() != 2);
    }
    if (coldadc_res) {
        glog.log("COLDADC configured\n");
    } else {
        glog.log("COLDADC configuration failed!\n");
    }
    
    if (adc_conf) {
        delete adc_conf;
        adc_conf = NULL;
    }

    // Pulser _must_ be off to program LArASIC
    bool pulser_res = set_pulser(false);
    
    bool larasic_res = true;
    uint32_t rx_mask = 0x0000;

    // FEMB number assignments for crpFEMBs[wibSlot][fembIdx]
    int crpFEMBs[6][4] = {{6,5,2,1}, {8,7,4,3}, {14,13,10,9}, {16,15,12,11}, {22,21,18,17}, {24,23,20,19}};
    for (int i = 0; i < 4; i++) {
        if (conf.fembs(i).enabled()) {
            larasic_conf c;
            memset(&c,0,sizeof(larasic_conf));
            
            const wib::ConfigureWIB::ConfigureFEMB &femb_conf = conf.fembs(i);
            
            c.sdd = femb_conf.buffer() == 2;
            c.sdc = femb_conf.ac_couple() == true;
            c.slkh = femb_conf.leak_10x() == true;
            c.slk = femb_conf.leak() == 1;
	    c.sgp = femb_conf.gain_match() == false;
            c.sdac = femb_conf.pulse_dac() & 0x3F;
            c.sdacsw2 = conf.pulser(); //connect pulser to channels
            
            c.sts = femb_conf.test_cap() == true;
            c.snc = femb_conf.baseline(); // New method - 0 = 900 mV, 1 = 200 mV, 2 = use APA mapping
            c.gain = femb_conf.gain() & 0x3;
            c.peak_time = femb_conf.peak_time() & 0x3;
            c.sdf = femb_conf.buffer() == 1;    
            
            c.cal_skip = femb_conf.strobe_skip();
            c.cal_delay = femb_conf.strobe_delay();
            c.cal_length = femb_conf.strobe_length();    

	    // FEMBs 13-24 are identical to 1-12 for CRP baseline mappings	    
            int fembNum = crpFEMBs[backplane_slot_num() & 0x7][i];
	    if (fembNum > 12) fembNum -= 12;
	    if (detector_type != 3) fembNum = 0; // FEMB number is currently irrelevant for non-CRP configurations
	    
            larasic_res &= femb[i]->configure_larasic(c, detector_type, fembNum); // Sets ACT to ACT_PROGRAM_LARASIC
        } else {
            rx_mask |= (0xF << (i*4));
        }
    }
    FEMB_3ASIC::fast_cmd(FAST_CMD_ACT); // Perform ACT
    if (larasic_res) {
        glog.log("LArASIC configured\n");
    } else {
        glog.log("LArASIC configuration failed!\n");
    }
    
    bool spi_verified = false;
    for (int i = 0; i < 10; i++) {
        usleep(10000);
        bool verify_res = true;
        for (int i = 0; i < 4; i++) {
            if (conf.fembs(i).enabled()) {
                verify_res &= femb[i]->set_fast_act(ACT_SAVE_STATUS);
            }
        }
        if (!verify_res) continue;
        FEMB_3ASIC::fast_cmd(FAST_CMD_ACT); // Perform ACT
        FEMB_3ASIC::fast_cmd(FAST_CMD_ACT); // Perform ACT
        for (int i = 0; i < 4; i++) {
            if (conf.fembs(i).enabled()) {
                verify_res &= femb[i]->read_spi_status();
            }
        }
        #ifdef SIMULATION
	glog.log("Monolithic board - Ignoring SPI check\n");
        verify_res = true;
        spi_verified = true;
        break;
        #endif
        if (verify_res) {
            spi_verified = true;
            break;
        } else {
            // Log which SPI programmings failed
            for (int i = 0; i < 4; i++) {
                if (conf.fembs(i).enabled()) {
                    femb[i]->log_spi_status();
                }
            }
        }
    }
    if (spi_verified) {
        glog.log("LArASIC SPI verified\n");
    } else {
        glog.log("LArASIC SPI verification failed!\n");
    }
    
    pulser_res &= set_pulser(conf.pulser());
        
    femb_rx_mask(rx_mask); 
    femb_rx_reset();
    glog.log("Serial receivers reset\n");

    if (calibrate()) {
      glog.log("ColdADCs calibrated\n");
    }
    else {
      glog.log("ColdADC calibration failed\n");
    }   

    enable_stamp_sync(true);
    reset_crc_bits();

    bool good_alignment = true;
    for (int i = 0; i < 4; i++) {
      if (conf.fembs(i).enabled()) {
	good_alignment &= check_alignment_delay(i); 
      }
    }

    
    return coldata_res && coldadc_res && larasic_res && spi_verified && pulser_res && good_alignment;
}

bool WIB_3ASIC::calibrate() {
    for (int i = 0; i < 4; i++) { //femb
        if (!frontend_power[i]) continue; // skip FEMBs that are off
        if (!femb[i]->setup_calib_auto()) {
            glog.log("Failed to calibrate FEMB %i\n",i);
            return false;
        }
	// femb[i]->dump_calib_constants();
    }
    return true;
}
