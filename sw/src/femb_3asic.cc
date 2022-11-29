#include "femb_3asic.h"

#include <unistd.h>
#include <cstdio>

constexpr size_t CD_I2C_ADDR[] = { 0xA0010000, 0xA0040000, 0xA0050000, 0xA0060000, 0xA0070000, 0xA0080000, 0xA0090000, 0xA00A0000 };

// for all coldata chips
constexpr size_t CD_FASTCMD_ADDR = 0xA0030000;
io_reg_t FEMB_3ASIC::coldata_fast_cmd;

FEMB_3ASIC::FEMB_3ASIC(int _index) : index(_index) {
    for (uint8_t i = 0; i < 2; i++) {
        io_reg_init(&this->coldata_i2c[i],CD_I2C_ADDR[i+index*2],2);
        last_coldata_i2c_chip[i] = -1;
    }
}

FEMB_3ASIC::~FEMB_3ASIC() {
    for (uint8_t i = 0; i < 2; i++) {
        io_reg_free(&this->coldata_i2c[i]);
    }
}

void FEMB_3ASIC::generate_coldata_register_values(bool cold, FrameType frame) {
    coldata_reg_conf.clear();

    uint8_t frame_value;
    switch (frame) {
        case FRAME_DD:
            frame_value = 3;
            break;
        case FRAME_12:
            frame_value = 0;
            break;
        case FRAME_14:
            frame_value = 1;
            break;
    }
    /* COLDATA configuration options to write 				*/
    /*										   ( page, 	addr, 	value)		*/
    coldata_reg_conf.push_back( std::make_tuple( 0, 	0x3, 	0x3c) );
    coldata_reg_conf.push_back( std::make_tuple( 0, 	0x11, 	0x7) );
    coldata_reg_conf.push_back( std::make_tuple( 0, 	0x25, 	0x40) );
    coldata_reg_conf.push_back( std::make_tuple( 0, 	0x27, 	0x1F) );

    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x48, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x49, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4a, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4b, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4c, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4d, 	0x0F) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4e, 	0x1) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x4f, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x50, 	0x0) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x51, 	0x0F) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x52, 	0x1) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x53, 	0x1) );
    coldata_reg_conf.push_back( std::make_tuple( 5, 	0x54, 	0x1) );
    
    coldata_reg_conf.push_back( std::make_tuple( 0, 	1, 		frame_value) );
    coldata_reg_conf.push_back( std::make_tuple( 0, 	3, 		0x3c) );
}

bool FEMB_3ASIC::reset_coldadc(uint8_t coldata_i) {
    return i2c_write_verify(0, coldata_i, 0, 0x20, ACT_RESET_COLDADC);
}

bool FEMB_3ASIC::configure_coldata(bool cold, FrameType frame) {
    bool res = true;
    //See COLDATA datasheet
    //See https://docs.google.com/document/d/1OAhVMvBe33dMkuIEOaqZNht0cjfUtLdNoIFy0QeGKp0/edit#

    generate_coldata_register_values(cold, frame);    

    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 2; i < 4; i++) { // For each COLDATA on FEMB
        for (auto entry : coldata_reg_conf) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            if (value != rvalue) res &= i2c_write_verify(0, i, page, addr, value);
        }
        res &= reset_coldadc(i);
    }
    if (!res) glog.log("COLDATA configuration failed for FEMB:%i!\n",index);
    return res;
}

bool FEMB_3ASIC::check_coldata_config() {
    bool res = true;

    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 2; i < 4; i++) { // For each COLDATA on FEMB
        for (auto entry : coldata_reg_conf) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            res &= (value == rvalue);
            if (value != rvalue) glog.log("COLDATA register check failed: COLDATA: %i, page: 0x%02X, address: 0x%02X, expected value: 0x%02X, read value: 0x%02X\n", i, page, addr, value, rvalue);
        }
    }
    return res;
}

void FEMB_3ASIC::generate_coldadc_register_values(bool cold, bool test_pattern = false, coldadc_conf *conf = NULL, bool se_larasic = true) {
    if (conf != NULL) glog.log("Using provided set of alternate COLDADC values\n");
    coldadc_reg_conf.clear();
    /* COLDADC configuration options to write 	*/
    /*					        			   ( page, 	addr, 	value)	*/
    coldadc_reg_conf.push_back( std::make_tuple( 2,		0x01, 	0x0c) );
    coldadc_reg_conf.push_back( std::make_tuple( 2, 	0x02, 	cold ? 0x7 : 0xF) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x80, 	(conf != NULL) ? conf->reg_0 : 0x23) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x84,	se_larasic ? 0x3b : 0x33) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x89, 	test_pattern ? 0x18 : 0x08) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x98, 	(conf != NULL) ? conf->reg_24 : 0xDF) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x99, 	(conf != NULL) ? conf->reg_25 : 0x33) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x9a, 	(conf != NULL) ? conf->reg_26 : 0x89) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x9b, 	(conf != NULL) ? conf->reg_27 : 0x67) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x9d, 	(conf != NULL) ? conf->reg_29 : 0x27) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x9e, 	(conf != NULL) ? conf->reg_30 : 0x27) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xb1, 	0x0c) );
    
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x81, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x82, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x83, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x85, 	0x33) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x86, 	0x33) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x87, 	0x33) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x88, 	0x0b) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8a, 	0xf1) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8b, 	0x29) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8c, 	0x8d) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8d, 	0x65) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8e, 	0x55) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x8f, 	0xff) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x90, 	0xff) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x91, 	0xff) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x92, 	0xff) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x93, 	0x04) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x94, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x95, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x96, 	0xff) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x97, 	0x2f) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0x9f, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa0, 	0x7f) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa1, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa2, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa3, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa4, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa5, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa6, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa7, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa8, 	0x00) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xa9, 	0x01) );
    coldadc_reg_conf.push_back( std::make_tuple( 1, 	0xaa, 	0x00) );
    
}

bool FEMB_3ASIC::configure_coldadc(bool cold, bool test_pattern, coldadc_conf *conf, bool se_larasic) {
    bool res = true;
    //See COLDADC datasheet
    //See https://docs.google.com/document/d/1OAhVMvBe33dMkuIEOaqZNht0cjfUtLdNoIFy0QeGKp0/edit#
    
    generate_coldadc_register_values(cold, test_pattern, conf, se_larasic);
    
    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 4; i <= 11; i++) { // Each COLDATA is now directly accessed through the second argument
        for (auto entry : coldadc_reg_conf) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            if (value != rvalue) res &= i2c_write_verify(0, i, page, addr, value);
        }
    }
    
    if (!res) glog.log("COLDADC configuration failed for FEMB:%i!\n",index);
    return res;
}

bool FEMB_3ASIC::check_coldadc_config() {
    bool res = true;

    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 4; i <= 11; i++) { // Each COLDATA is now directly accessed through the second argument
        for (auto entry : coldadc_reg_conf) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            res &= (value == rvalue);
            if (value != rvalue) glog.log("COLDADC register check failed: COLDATA: %i, page: 0x%02X, address: 0x%02X, expected value: 0x%02X, read value: 0x%02X\n", i, page, addr, value, rvalue);
        }
    }
    return res;
}

bool FEMB_3ASIC::setup_calib_auto() {
    bool res = true;
    for (uint8_t j = 4; j <= 11; j++) { // For each COLADC attached to COLDATA
        glog.log("Starting calibration on COLDADC:%i!\n",j);
        res &= i2c_write_verify(0, j, 1, 0x80+41, 0x00);
        res &= i2c_write_verify(0, j, 1, 0x80+31, 0x03);
        usleep(500000);
        res &= i2c_write_verify(0, j, 1, 0x80+31, 0x00);
        res &= i2c_write_verify(0, j, 1, 0x80+41, 0x01);
        glog.log("Done calibration on COLDADC:%i!\n",j);
    }
    return res;
}

// must run sn = 0 first, then 1,2,3; any other sn resets
// must run stage 6 first, ..., stage 0
bool FEMB_3ASIC::setup_calib(uint8_t sn, uint8_t stage) {
    bool res = true;
    //Based on algorithm from Dave Christian
    //See COLDADC datasheet
    if (stage > 6) return false; //invalid
    for (uint8_t i = 4; i <= 11; i++) { // Each COLDATA is now directly accessed through the second argument
        switch (sn) {
            case 0: {
                const uint8_t cal_stages = (stage+1) & 0x7;
                res &= i2c_write_verify(0, i, 1, 0x80+32, (cal_stages<<4)|0xF); //set number of stages
                res &= i2c_write_verify(0, i, 1, 0x80+44, (stage<<2)|0x3); //enable forcing for stage (both pipelines)
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x30); //clear calib registers (both pipelines)
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x00);
                } break; //acquire s0
            case 1:
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x03);
                break; //acquire s1
            case 2:
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x0f);
                break; //acquire s2
            case 3:
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x0c);
                res &= i2c_write_verify(0, i, 1, 0x80+45, 0x30);
                break; //acquire s3
            default:
                res &= i2c_write_verify(0, i, 1, 0x80+44, 0x00);
                res &= i2c_write_verify(0, i, 1, 0x80+45, 0x00);
                res &= i2c_write_verify(0, i, 1, 0x80+46, 0x00);
        }
    }
    return res;
}

bool FEMB_3ASIC::store_calib(const uint16_t w0_vals[8][2], const uint16_t w2_vals[8][2], uint8_t stage) {
    bool res = true;
    //Based on algorithm from Dave Christian
    //See COLDADC datasheet
    if (stage > 6) return false; //invalid
    for (uint8_t i = 4; i <= 11; i++) { // Each COLDATA is now directly accessed through the second argument
        uint8_t j = i-4; // coldadc index in w* arrays
        for (uint8_t adc = 0; adc < 2; adc++) { // 0 = pipeline 0; 1 = pipeline 1
            for (uint8_t byte = 0; byte < 2; byte++) { // 0 = low byte; 1 = high byte
                uint8_t w0_addr = (adc<<6)|(stage<<1)|byte;
                res &= i2c_write_verify(0, i, 1, w0_addr, (w0_vals[j][adc]>>(8*byte))&0xFF);
                uint8_t w2_addr = (adc<<6)|0x20|(stage<<1)|byte;
                res &= i2c_write_verify(0, i, 1, w2_addr, (w2_vals[j][adc]>>(8*byte))&0xFF);
            }
        }
    }
    return res;
}

void FEMB_3ASIC::generate_larasic_register_values(const larasic_conf &c) {
    larasic_reg_conf.clear();
    
    uint8_t global_reg_1 = ((c.sgp ? 1 : 0) << 7) //SGP bit for DAC gain matching (0 is enabled, 1 is disabled)
            | ((c.sdd ? 1 : 0) << 6) // 1 = "SEDC" buffer enabled
            | ((c.sdc ? 1 : 0) << 5) // 0 = dc; 1 = ac
            | ((c.slkh ? 1 : 0) << 4) // 1 = "RQI" * 10 enable
            | ((c.s16 ? 1 : 0) << 3) // 1 = ch15 high filter enable
            | ((c.stb ? 1 : 0) << 2) // 0 = mon analog channel; 1 = use stb1
            | ((c.stb1 ? 1 : 0) << 1) // 0 = mon temp; 1 = mon bandgap
            | ((c.slk ? 1 : 0) << 0); // 0 = 500 pA RQI; 1 = 100 pA RQI
                         
    uint8_t global_reg_2 = (((c.sdac & 0x20) >> 5) << 2) // 6 bit current scaling daq reversed here
            | (((c.sdac & 0x10) >> 4) << 3)
            | (((c.sdac & 0x08) >> 3) << 4)
            | (((c.sdac & 0x04) >> 2) << 5)
            | (((c.sdac & 0x02) >> 1) << 6)
            | (((c.sdac & 0x01) >> 0) << 7)
            | ((c.sdacsw1 ? 1 : 0) << 1) // 1 = connected to external test pin
            | ((c.sdacsw2 ? 1 : 0) << 0); // 1 = connected to DAC output
                         
    uint8_t channel_reg = ((c.sts ? 1 : 0) << 7) // 1 = test capacitor enabled
            | ((c.snc ? 1 : 0) << 6) // 0 = 900 mV baseline;1 = 200 mV baseline
            | ((c.gain & 0x1) << 5)
            | ((c.gain & 0x2) << 3) // 14, 25, 7.8, 4.7 mV/fC (0 - 3) reversed here
            | ((c.peak_time & 0x1) << 3)
            | ((c.peak_time & 0x2) << 1) // 1.0, 0.5, 3, 2 us (0 - 3) reversed here
            | ((c.smn ? 1 : 0) << 1) // 1 = monitor enable
            | ((c.sdf ? 1 : 0) << 0); // 1 = "SE" buffer enable

    // See COLDATA datasheet
    // MSB goes first
    // [MSB] Ch15 .. Ch0 global_reg_1 global_reg_2 [LSB]
    // COLDATA registers 0x80 .. 0x91

	for (uint8_t i = 2; i < 4; i++) { // For each COLDATA on FEMB
        for (uint8_t page = 1; page <= 4; page++) { // For each LArASIC page in COLDATA
            for (uint8_t addr = 0x82; addr < 0x92; addr++) { // set channel registers
                if (c.snc == 2) {		
                    int chip = (i-2)*4 + page - 1;
                    channel_reg = ((c.sts ? 1 : 0) << 7) // 1 = test capacitor enabled
                            | ((APABaselineMapping[chip][addr-0x82]) << 6) // 0 = 900 mV baseline;1 = 200 mV baseline
                            | ((c.gain & 0x1) << 5)
                            | ((c.gain & 0x2) << 3) // 14, 25, 7.8, 4.7 mV/fC (0 - 3) reversed here
                            | ((c.peak_time & 0x1) << 3)
                            | ((c.peak_time & 0x2) << 1) // 1.0, 0.5, 3, 2 us (0 - 3) reversed here
                            | ((c.smn ? 1 : 0) << 1) // 1 = monitor enable
                            | ((c.sdf ? 1 : 0) << 0); // 1 = "SE" buffer enable	      
                }
                /*					        	              ( page, addr, value)	*/
                larasic_reg_conf[i].push_back( std::make_tuple( page, addr, channel_reg) );
            }
            larasic_reg_conf[i].push_back(     std::make_tuple( page, 0x80, global_reg_2) );
            larasic_reg_conf[i].push_back(     std::make_tuple( page, 0x81, global_reg_1) );
            
            // COLDATA calibration stobe parameters
            larasic_reg_conf[i].push_back(     std::make_tuple( page, 0x06, c.cal_skip) );
            larasic_reg_conf[i].push_back(     std::make_tuple( page, 0x07, c.cal_delay) );
            larasic_reg_conf[i].push_back(     std::make_tuple( page, 0x08, c.cal_length) );
        }
    }
}

bool FEMB_3ASIC::configure_larasic(const larasic_conf &c) {
    bool res = true;

    generate_larasic_register_values(c);

    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 2; i < 4; i++) { // For each COLDATA on FEMB
        for (auto entry : larasic_reg_conf.at(i)) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            if (value != rvalue) res &= i2c_write_verify(0, i, page, addr, value);
        }
        set_fast_act(ACT_PROGRAM_LARASIC);
    }
    
    if (!res) glog.log("Failed to store LArASIC configuration for FEMB:%i!\n",index);
    return res;
}

bool FEMB_3ASIC::check_larasic_config() {
    bool res = true;

    uint8_t page, addr, value, rvalue;
    for (uint8_t i = 2; i < 4; i++) { // For each COLDATA on FEMB
        for (auto entry : larasic_reg_conf.at(i)) {
            std::tie(page, addr, value) = entry;
            rvalue = i2c_read(0, i, page, addr);
            res &= (value == rvalue);
            if (value != rvalue) glog.log("LArASIC register check failed: COLDATA: %i, page: 0x%02X, address: 0x%02X, expected value: 0x%02X, read value: 0x%02X\n", i, page, addr, value, rvalue);
        }
    }
    return res;
}

bool FEMB_3ASIC::set_fast_act(uint8_t act_cmd) {
    bool res = true;
    for (uint8_t i = 2; i < 4; i++) {
        res &= i2c_write_verify(0, i, 0, 0x20, act_cmd);
    }
    //glog.log("Fast command buffer is now %lx\n",act_cmd);
    if (!res) glog.log("Failed to set fast act for FEMB:%i!\n",index);
    return res;
}

void FEMB_3ASIC::log_spi_status() {
    bool res = true;
    for (uint8_t i = 2; i < 4; i++) {
        uint8_t status = i2c_read(0,i,0,0x24);
        glog.log("status is 0x%02X\n", status);
        for (uint8_t j = 0; j < 4; j++) {
            glog.log("the way it gets it is %i and %i\n",(j*2), (j*2+1));
            bool done = (status >> (j*2+0)) & 0x1 == 0x1;
            bool success = (status >> (j*2+1)) & 0x1 == 0x1;
            if (!done || !success) {
                glog.log("Failed to verify SPI for FEMB:%i LArASIC:%i\n",index,j*2+1);
            }
        }
    }
}


bool FEMB_3ASIC::read_spi_status() {
    bool res = true;
    for (uint8_t i = 2; i < 4; i++) {
        uint8_t status = i2c_read(0,i,0,0x24);
        res &= (status == 0xFF); // all bits 1 for success
    }
    return res;
}

bool FEMB_3ASIC::set_control_reg(uint8_t coldata_idx, bool ctrl_0, bool ctrl_1) {
    return i2c_write_verify(0,coldata_idx,0,0x26,(ctrl_1?2:0)|(ctrl_0?1:0));
}

void FEMB_3ASIC::fast_cmd(uint8_t cmd_code) {
    static bool fast_cmd_init = false;
    if (!fast_cmd_init) {
        io_reg_init(&FEMB_3ASIC::coldata_fast_cmd,CD_FASTCMD_ADDR,2); //never free'd
        io_reg_write(&FEMB_3ASIC::coldata_fast_cmd,REG_FAST_CMD_ACT_DELAY,19);
        fast_cmd_init = true;
    }
    //glog.log("Wrote a fast command of %d\n",cmd_code);
    io_reg_write(&FEMB_3ASIC::coldata_fast_cmd,REG_FAST_CMD_CODE,cmd_code);
}

void FEMB_3ASIC::i2c_bugfix(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    if (last_coldata_i2c_chip[bus_idx] != chip_addr) { // Coldata i2c bug latching chip_addr 
        last_coldata_i2c_chip[bus_idx] = chip_addr;
        i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
        i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
    }
}

void FEMB_3ASIC::i2c_write(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data) {
    //i2c_bugfix(bus_idx,chip_addr,reg_page,reg_addr);
    uint32_t ctrl = ((chip_addr & 0xF) << COLD_I2C_CHIP_ADDR)
                  | ((reg_page & 0x7) << COLD_I2C_REG_PAGE)
                  | (0x0 << COLD_I2C_RW)
                  | ((reg_addr & 0xFF) << COLD_I2C_REG_ADDR)
                  | ((data & 0xFF) << COLD_I2C_DATA);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_CTRL,ctrl);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_START,1);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_START,0);
    usleep(COLD_I2C_DELAY);
}

uint8_t FEMB_3ASIC::i2c_read(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    //i2c_bugfix(bus_idx,chip_addr,reg_page,reg_addr);     
    uint32_t ctrl = ((chip_addr & 0xF) << COLD_I2C_CHIP_ADDR)
                  | ((reg_page & 0x7) << COLD_I2C_REG_PAGE)
                  | (0x1 << COLD_I2C_RW)
                  | ((reg_addr & 0xFF) << COLD_I2C_REG_ADDR);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_CTRL,ctrl);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_START,1);
    io_reg_write(&this->coldata_i2c[bus_idx],REG_COLD_I2C_START,0);
    usleep(COLD_I2C_DELAY);
    ctrl = io_reg_read(&this->coldata_i2c[bus_idx],REG_COLD_I2C_CTRL);
    // fix for C2W signal inversion in COLDATA chip #0
    //if (bus_idx == 0) ctrl = ~ctrl;
    return (ctrl >> COLD_I2C_DATA) & 0xFF;
}

bool FEMB_3ASIC::i2c_write_verify(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data, size_t retries) {
    #ifdef SIMULATION
    glog.log("simulated i2c_write_verify FEMB:%i COLDATA:%i chip:0x%X page:0x%X reg:0x%02X :: 0x%02X\n",index,bus_idx,chip_addr,reg_page,reg_addr,data);
    return true;
    #else
    uint8_t read;
    for (size_t i = 0; i <= retries; i++) {
        i2c_write(bus_idx,chip_addr,reg_page,reg_addr,data);
        read = i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
        if ((read & 0xFF) == data) return true;
    }
    glog.log("read is 0x%02X\n",read);
    glog.log("complimentary read is 0x%02X\n",~read & 0xFF);
    glog.log("i2c_write_verify failed FEMB:%i COLDATA:%i chip:0x%X page:0x%X reg:0x%02X :: 0x%02X != 0x%02X\n",index,bus_idx,chip_addr,reg_page,reg_addr,data,read);
    return false;
    #endif
}
