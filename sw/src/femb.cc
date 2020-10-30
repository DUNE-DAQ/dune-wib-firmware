#include "femb.h"

#include <unistd.h>

constexpr size_t CD_I2C_ADDR[] = { 0xA0010000, 0xA0040000, 0xA0050000, 0xA0060000, 0xA0070000, 0xA0080000, 0xA0090000, 0xA00A0000 };

// for all coldata chips
constexpr size_t CD_FASTCMD_ADDR = 0xA0030000;
io_reg_t FEMB::coldata_fast_cmd;

FEMB::FEMB(int index) {
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

void FEMB::configure_coldata() {
    //See COLDATA datasheet
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
		i2c_write(i, 2, 5, 0x40, 0x3);    //CONFIG_PLL_ICP
		i2c_write(i, 2, 5, 0x41, 0x10);	//CONFIG_PLL_BAND
		i2c_write(i, 2, 5, 0x42, 0x2);	//CONFIG_PLL_LPFR
		i2c_write(i, 2, 5, 0x43, 0x2);	//CONFIG_PLL_ATO
		i2c_write(i, 2, 5, 0x44, 0x0);	//CONFIG_PLL_PDCP
		i2c_write(i, 2, 5, 0x45, 0x0);	//CONFIG_PLL_OPEN

		i2c_write(i, 2, 5, 0x46, 0x1);	//CONFIG_SER_MODE
		i2c_write(i, 2, 5, 0x47, 0x0);	//CONFIG_SER_INV_SER_CLK

		i2c_write(i, 2, 5, 0x48, 0x0);	//CONFIG_DRV_VMBOOST
		i2c_write(i, 2, 5, 0x49, 0x0);	//CONFIG_DRV_VMDRIVER
		i2c_write(i, 2, 5, 0x4a, 0x0);	//CONFIG_DRV_SELPRE
		i2c_write(i, 2, 5, 0x4b, 0x0);	//CONFIG_DRV_SELPST1
		i2c_write(i, 2, 5, 0x4c, 0x0);	//CONFIG_DRV_SELPST2
		i2c_write(i, 2, 5, 0x4d, 0x0F);	//CONFIG_DRV_SELCM_MAIN
		i2c_write(i, 2, 5, 0x4e, 0x1);	//CONFIG_DRV_ENABLE_CM
		i2c_write(i, 2, 5, 0x4f, 0x0);	//CONFIG_DRV_INVERSE_CLK
		i2c_write(i, 2, 5, 0x50, 0x0);	//CONFIG_DRV_DELAYSEL
		i2c_write(i, 2, 5, 0x51, 0x0F);	//CONFIG_DRV_DELAY_CS
		i2c_write(i, 2, 5, 0x52, 0x1);	//CONFIG_DRV_CML
		i2c_write(i, 2, 5, 0x53, 0x1);	//CONGIF_DRV_BIAS_CML_INTERNAL
		i2c_write(i, 2, 5, 0x54, 0x1);	//CONGIF_DRV_BIAS_CS_INTERNAL

		//i2c_write(i, 2, 0, 1, 0);  // frame-12
		//i2c_write(i, 2, 0, 1, 1);  // frame-14
		i2c_write(i, 2, 0, 1, 3);  // frame-DD
		//i2c_write (i, 2, 0, 3, 0xc3);  // PRBS7, no 8b10b
		i2c_write(i, 2, 0, 3, 0x3c);  // normal operation
		i2c_write(i, 2, 0, 0x20, ACT_RESET_COLDADC); // ACT = COLDADC reset
	}
}

void FEMB::configure_coldadc() {
    //See COLDADC datasheet
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
        for (uint8_t j = 4; j <= 7; j++) { // For each COLADC attached to COLDATA
            i2c_write(i, j, 2, 0x01, 0x0c);  //start_data
            i2c_write(i, j, 1, 0x96, 0xff);  //bjt_powerdown
            i2c_write(i, j, 1, 0x97, 0x2f);  //ref_bias
            i2c_write(i, j, 1, 0x93, 0x04);  //internal_ref
            i2c_write(i, j, 1, 0x9C, 0x15);  //vt45uA
            i2c_write(i, j, 1, 0x98, 0xFF);  //vrefp
            i2c_write(i, j, 1, 0x99, 0x00);  //vrefn
            i2c_write(i, j, 1, 0x9a, 0x80);  //vcmo
            i2c_write(i, j, 1, 0x9b, 0x60);  //vcmi
            i2c_write(i, j, 1, 0x9d, 0x27);  //ref-bias
            i2c_write(i, j, 1, 0x9e, 0x27);  //ref-bias
            i2c_write(i, j, 1, 0x80, 0x63);  //sdc_bypassed
            i2c_write(i, j, 1, 0x84, 0x3b);  //single-ened_input_mode
            i2c_write(i, j, 1, 0x88, 0x0b);  //ADC-bias-current-50uA
            i2c_write(i, j, 1, 0x89, 0x08);  //offset_binary_output_data_format
            i2c_write(i, j, 1, 0x89, 0x08);  //offset_binary_output_data_format
        }
    }
}

void FEMB::configure_larasic(const larasic_conf &c) {

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
                        | ((c.smn ? 1 : 0) << 1) // 1 = monitor enable
                        | ((c.sdf ? 1 : 0) << 1); // 1 = "SE" buffer enable
                        
    // See COLDATA datasheet
    // MSB goes first
    // [MSB] Ch15 .. Ch0 global_reg_1 global_reg_2 [LSB]
    // COLDATA registers 0x80 .. 0x91
    
    for (uint8_t i = 0; i < 2; i++) { // For each COLDATA on FEMB
        for (uint8_t page = 1; page <= 4; page++) { // For each LArASIC page in COLDATA
            for (uint8_t addr = 0x80; addr < 0x90; addr++) { // set channel registers
                i2c_write(i, 2, page, addr, channel_reg); 
            }
            i2c_write(i, 2, page, 0x90, global_reg_1);
            i2c_write(i, 2, page, 0x91, global_reg_2); 
        }
		i2c_write(i, 2, 0, 0x20, ACT_PROGRAM_LARASIC); // ACT = COLDADC reset
    }
    
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
    if (last_coldata_i2c_chip[bus_idx] != chip_addr) { // Bug #2
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
    usleep(27);
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
    usleep(27);
    ctrl = io_reg_read(&this->coldata_i2c[bus_idx],REG_COLD_I2C_CTRL);
    return (ctrl >> COLD_I2C_DATA) & 0xFF;
}
