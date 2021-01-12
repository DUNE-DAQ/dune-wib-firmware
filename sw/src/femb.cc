#include "femb.h"

#include <unistd.h>
#include <cstdio>

constexpr size_t CD_I2C_ADDR[] = { 0xA0010000, 0xA0040000, 0xA0050000, 0xA0060000, 0xA0070000, 0xA0080000, 0xA0090000, 0xA00A0000 };

// for all coldata chips
constexpr size_t CD_FASTCMD_ADDR = 0xA0030000;
io_reg_t FEMB::coldata_fast_cmd;

FEMB::FEMB(int _index) : index(_index) {
    for (uint8_t i = 0; i < 2; i++) {
        io_reg_init(&this->coldata_i2c[i],CD_I2C_ADDR[i+index*2],2);
        last_coldata_i2c_chip[i] = -1;
    }
}

FEMB::~FEMB() {
    for (uint8_t i = 0; i < 2; i++) {
        io_reg_free(&this->coldata_i2c[i]);
    }
}

bool FEMB::configure_coldata(bool cold, FrameType frame) {
    bool res = true;
    //See COLDATA datasheet
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
        res &= i2c_write_verify(i, 2, 5, 0x40, 0x3);    //CONFIG_PLL_ICP
        res &= i2c_write_verify(i, 2, 5, 0x41, cold ? 0x08 : 0x10);    //CONFIG_PLL_BAND
        res &= i2c_write_verify(i, 2, 5, 0x42, 0x2);    //CONFIG_PLL_LPFR
        res &= i2c_write_verify(i, 2, 5, 0x43, 0x2);    //CONFIG_PLL_ATO
        res &= i2c_write_verify(i, 2, 5, 0x44, 0x0);    //CONFIG_PLL_PDCP
        res &= i2c_write_verify(i, 2, 5, 0x45, 0x0);    //CONFIG_PLL_OPEN

        res &= i2c_write_verify(i, 2, 5, 0x46, 0x1);    //CONFIG_SER_MODE
        res &= i2c_write_verify(i, 2, 5, 0x47, 0x0);    //CONFIG_SER_INV_SER_CLK
        
        res &= i2c_write_verify(i, 2, 5, 0x48, 0x0);    //CONFIG_DRV_VMBOOST
        //25m cable values
        //res &= i2c_write_verify(i, 2, 5, 0x48, cold ? 0x3 : 0x7);    //CONFIG_DRV_VMBOOST
        res &= i2c_write_verify(i, 2, 5, 0x49, 0x0);    //CONFIG_DRV_VMDRIVER
        
        res &= i2c_write_verify(i, 2, 5, 0x4a, 0x0);    //CONFIG_DRV_SELPRE
        res &= i2c_write_verify(i, 2, 5, 0x4b, 0x0);    //CONFIG_DRV_SELPST1
        res &= i2c_write_verify(i, 2, 5, 0x4c, 0x0);    //CONFIG_DRV_SELPST2
        //25m cable values
        //res &= i2c_write_verify(i, 2, 5, 0x4a, cold ? 0x0 : 0x1);    //CONFIG_DRV_SELPRE
        //res &= i2c_write_verify(i, 2, 5, 0x4b, cold ? 0x2 : 0xA);    //CONFIG_DRV_SELPST1
        //res &= i2c_write_verify(i, 2, 5, 0x4c, cold ? 0x0 : 0x1);    //CONFIG_DRV_SELPST2
        res &= i2c_write_verify(i, 2, 5, 0x4d, 0x0F);    //CONFIG_DRV_SELCM_MAIN
        res &= i2c_write_verify(i, 2, 5, 0x4e, 0x1);    //CONFIG_DRV_ENABLE_CM
        res &= i2c_write_verify(i, 2, 5, 0x4f, 0x0);    //CONFIG_DRV_INVERSE_CLK
        res &= i2c_write_verify(i, 2, 5, 0x50, 0x0);    //CONFIG_DRV_DELAYSEL
        res &= i2c_write_verify(i, 2, 5, 0x51, 0x0F);    //CONFIG_DRV_DELAY_CS
        res &= i2c_write_verify(i, 2, 5, 0x52, 0x1);    //CONFIG_DRV_CML
        res &= i2c_write_verify(i, 2, 5, 0x53, 0x1);    //CONGIF_DRV_BIAS_CML_INTERNAL
        res &= i2c_write_verify(i, 2, 5, 0x54, 0x1);    //CONGIF_DRV_BIAS_CS_INTERNAL
        
        switch (frame) {
            case FRAME_DD:
                res &= i2c_write_verify(i, 2, 0, 1, 3);
                break;
            case FRAME_12:
                res &= i2c_write_verify(i, 2, 0, 1, 0);
                break;
            case FRAME_14:
                res &= i2c_write_verify(i, 2, 0, 1, 1);
                break;
        }
        
        
        
        //i2c_write (i, 2, 0, 3, 0xc3);  // PRBS7, no 8b10b
        res &= i2c_write_verify(i, 2, 0, 3, 0x3c);  // normal operation
        res &= i2c_write_verify(i, 2, 0, 0x20, ACT_RESET_COLDADC); // ACT = COLDADC reset
    }
    if (!res) glog.log("COLDATA configuration failed for FEMB:%i!\n",index);
    return res;
}

bool FEMB::configure_coldadc(bool cold, bool test_pattern) {
    bool res = true;
    //See COLDADC datasheet
    //FIXME do these options need to be configurable?
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
        for (uint8_t j = 4; j <= 7; j++) { // For each COLADC attached to COLDATA
            res &= i2c_write_verify(i, j, 2, 0x01, 0x0c);  //start_data
            res &= i2c_write_verify(i, j, 2, 0x02, cold ? 0x7 : 0xF);  //lvds_current
            res &= i2c_write_verify(i, j, 1, 0x96, 0xff);  //bjt_powerdown
            res &= i2c_write_verify(i, j, 1, 0x97, 0x2f);  //ref_bias
            res &= i2c_write_verify(i, j, 1, 0x93, 0x04);  //internal_ref
            res &= i2c_write_verify(i, j, 1, 0x9C, 0x15);  //vt45uA
            res &= i2c_write_verify(i, j, 1, 0x98, cold ? 0xDF : 0xFF);  //vrefp //shanshan cold 0xE0
            res &= i2c_write_verify(i, j, 1, 0x99, cold ? 0x33 : 0x00);  //vrefn //shanshan cold 0x10
            res &= i2c_write_verify(i, j, 1, 0x9a, cold ? 0x89 : 0x80);  //vcmo  //shanshan cold 0x87
            res &= i2c_write_verify(i, j, 1, 0x9b, cold ? 0x67 : 0x60);  //vcmi  //shanshan cold 0x60
            res &= i2c_write_verify(i, j, 1, 0x9d, 0x27);  //ref-bias
            res &= i2c_write_verify(i, j, 1, 0x9e, 0x27);  //ref-bias
            res &= i2c_write_verify(i, j, 1, 0x80, 0x63);  //sdc_bypassed
            res &= i2c_write_verify(i, j, 1, 0x84, 0x3b);  //single-ened_input_mode
            res &= i2c_write_verify(i, j, 1, 0x88, 0x0b);  //ADC-bias-current-50uA
            res &= i2c_write_verify(i, j, 1, 0x89, test_pattern ? 0x18 : 0x08);  //offset_binary_output_data_format
            res &= i2c_write_verify(i, j, 1, 0xb1, 0x0c);  //config_start_number, as recommended by David
        }
    }
    if (!res) glog.log("COLDADC configuration failed for FEMB:%i!\n",index);
    return res;
}

bool FEMB::configure_larasic(const larasic_conf &c) {
    bool res = true;

    // See LArASIC datasheet
    uint8_t global_reg_1 = ((c.sdd ? 1 : 0) << 1) // 1 = "SEDC" buffer enabled
                         | ((c.sdc ? 1 : 0) << 2) // 0 = dc; 1 = ac
                         | ((c.slkh ? 1 : 0) << 3) // 1 = "RQI" * 10 enable
                         | ((c.s16 ? 1 : 0) << 4) // 1 = ch15 high filter enable
                         | ((c.stb ? 1 : 0) << 5) // 0 = mon analog channel; 1 = use stb1
                         | ((c.stb1 ? 1 : 0) << 6) // 0 = mon temp; 1 = mon bandgap
                         | ((c.slk ? 1 : 0) << 7); // 0 = 500 pA RQI; 1 = 100 pA RQI
                         
    uint8_t global_reg_2 = ((c.sdac & 0x3F) << 0) // 6 bit current scaling daq 
                         | ((c.sdacsw1 ? 1 : 0) << 6) // 1 = connected to external test pin
                         | ((c.sdacsw2 ? 1 : 0) << 7); // 1 = connected to DAC output
                         
    uint8_t channel_reg = ((c.sts ? 1 : 0) << 0) // 1 = test capacitor enabled
                        | ((c.snc ? 1 : 0) << 1) // 0 = 900 mV baseline;1 = 200 mV baseline
                        | ((c.gain & 0x3) << 2) // 14, 25, 7.8, 4.7 mV/fC (0 - 3)
                        | ((c.peak_time & 0x3) << 4) // 1.0, 0.5, 3, 2 us (0 - 3)
                        | ((c.smn ? 1 : 0) << 6) // 1 = monitor enable
                        | ((c.sdf ? 1 : 0) << 7); // 1 = "SE" buffer enable
                        
    // See COLDATA datasheet
    // MSB goes first
    // [MSB] Ch15 .. Ch0 global_reg_1 global_reg_2 [LSB]
    // COLDATA registers 0x80 .. 0x91
    
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
        for (uint8_t page = 1; page <= 4; page++) { // For each LArASIC page in COLDATA
            for (uint8_t addr = 0x80; addr < 0x90; addr++) { // set channel registers
                res &= i2c_write_verify(i, 2, page, addr, channel_reg); 
            }
            res &= i2c_write_verify(i, 2, page, 0x90, global_reg_1);
            res &= i2c_write_verify(i, 2, page, 0x91, global_reg_2); 
            
            // COLDATA calibration stobe parameters
            res &= i2c_write_verify(i, 2, page, 0x06, c.cal_skip); 
            res &= i2c_write_verify(i, 2, page, 0x07, c.cal_delay);
            res &= i2c_write_verify(i, 2, page, 0x08, c.cal_length);  
        }
        res &= i2c_write_verify(i, 2, 0, 0x20, ACT_PROGRAM_LARASIC); // ACT = Program LArASIC SPI
    }
    
    if (!res) glog.log("Failed to store LArASIC configuration for FEMB:%i!\n",index);
    return res;
}

bool FEMB::set_fast_act(uint8_t act_cmd) {
    bool res = true;
    for (uint8_t i = 0; i < 2; i++) {
        res &= i2c_write_verify(i, 2, 0, 0x20, act_cmd);
    }
    if (!res) glog.log("Failed to set fast act for FEMB:%i!\n",index);
    return res;
}

bool FEMB::read_spi_status() {
    bool res = true;
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t status = i2c_read(i,2,0,0x23);
        res &= (status == 0xFF); // all bits 1 for success
    }
    return res;
}

bool FEMB::set_control_reg(uint8_t coldata_idx, bool ctrl_0, bool ctrl_1) {
    return i2c_write_verify(coldata_idx,2,0,0x25,(ctrl_1?2:0)|(ctrl_0?1:0));
}

void FEMB::fast_cmd(uint8_t cmd_code) {
    static bool fast_cmd_init = false;
    if (!fast_cmd_init) {
        io_reg_init(&FEMB::coldata_fast_cmd,CD_FASTCMD_ADDR,2); //never free'd
        io_reg_write(&FEMB::coldata_fast_cmd,REG_FAST_CMD_ACT_DELAY,19);
        fast_cmd_init = true;
    }
    io_reg_write(&FEMB::coldata_fast_cmd,REG_FAST_CMD_CODE,cmd_code);
}

void FEMB::i2c_bugfix(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    if (last_coldata_i2c_chip[bus_idx] != chip_addr) { // Coldata i2c bug latching chip_addr 
        last_coldata_i2c_chip[bus_idx] = chip_addr;
        i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
        i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
    }
}

void FEMB::i2c_write(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data) {
    i2c_bugfix(bus_idx,chip_addr,reg_page,reg_addr);
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

uint8_t FEMB::i2c_read(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    i2c_bugfix(bus_idx,chip_addr,reg_page,reg_addr);    
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
    if (bus_idx == 0) ctrl = ~ctrl;
    return (ctrl >> COLD_I2C_DATA) & 0xFF;
}

bool FEMB::i2c_write_verify(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data, size_t retries) {
    uint8_t read;
    for (size_t i = 0; i <= retries; i++) {
        i2c_write(bus_idx,chip_addr,reg_page,reg_addr,data);
        read = i2c_read(bus_idx,chip_addr,reg_page,reg_addr);
        if (read == data) return true;
    }
    glog.log("i2c_write_verify failed FEMB:%i COLDATA:%i chip:0x%X page:0x%X reg:0x%X :: 0x%X != 0x%X\n",index,bus_idx,chip_addr,reg_page,reg_addr,data,read);
    return false;
}

