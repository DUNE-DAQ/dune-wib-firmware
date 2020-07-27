#include "femb.h"

#include <unistd.h>

FEMB::FEMB(int index) {
    for (int i = 0; i < 2; i++) {
        io_reg_init(&this->coldata_i2c[i],0xA0010000+0x40000*(i+2*index),2);
        io_reg_init(&this->coldata_fast_cmd[i],0xA0030000+0x40000*(i+2*index),2);
        io_reg_write(&this->coldata_fast_cmd[i],REG_FAST_CMD_ACT_DELAY,19);
        last_coldata_i2c_chip[i] = -1;
    }
}

FEMB::~FEMB() {
    for (int i = 0; i < 2; i++) {
        io_reg_free(&this->coldata_i2c[i]);
        io_reg_free(&this->coldata_fast_cmd[i]);
    }
}


void FEMB::fast_cmd(int coldata_idx, uint8_t cmd_code) {
    io_reg_write(&this->coldata_fast_cmd[coldata_idx],REG_FAST_CMD_CODE,cmd_code);
}

void FEMB::i2c_bugfix(int coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    if (last_coldata_i2c_chip[coldata_idx] != chip_addr) { // Bug #2
        i2c_read(coldata_idx,chip_addr,reg_page,reg_addr);
        i2c_read(coldata_idx,chip_addr,reg_page,reg_addr);
        last_coldata_i2c_chip[coldata_idx] = chip_addr;
    }
}

void FEMB::i2c_write(int coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data) {
    i2c_bugfix(coldata_idx,chip_addr,reg_page,reg_addr);
    uint32_t ctrl = ((chip_addr & 0xF) << COLD_I2C_CHIP_ADDR)
                  | ((reg_page & 0x7) << COLD_I2C_REG_PAGE)
                  | (0x0 << COLD_I2C_RW)
                  | ((reg_addr & 0xF) << COLD_I2C_REG_ADDR)
                  | ((data & 0xF) << COLD_I2C_DATA);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_CTRL,ctrl);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_START,1);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_START,0);
    usleep(27);
}


uint8_t FEMB::i2c_read(int coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    i2c_bugfix(coldata_idx,chip_addr,reg_page,reg_addr);    
    uint32_t ctrl = ((chip_addr & 0xF) << COLD_I2C_CHIP_ADDR)
                  | ((reg_page & 0x7) << COLD_I2C_REG_PAGE)
                  | (0x1 << COLD_I2C_RW)
                  | ((reg_addr & 0xF) << COLD_I2C_REG_ADDR);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_CTRL,ctrl);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_START,1);
    io_reg_write(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_START,0);
    usleep(27);
    ctrl = io_reg_read(&this->coldata_i2c[coldata_idx],REG_COLD_I2C_CTRL);
    return (ctrl >> COLD_I2C_DATA) & 0xF;
}
