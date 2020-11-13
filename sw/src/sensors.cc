#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "sensors.h"
#include "i2c.h"
#include "io_reg.h"

void start_ltc2499_temp(i2c_t *i2c, uint8_t ch) {
    uint8_t cmd[2] = { (uint8_t)(0xB0 | ((ch%2)<<3) | (ch/2)), 0x80};
    i2c_write(i2c,0x15,cmd,2);
}

double read_ltc2499_temp(i2c_t *i2c, uint8_t next_ch) {
    uint8_t cmd[2] = { (uint8_t)(0xB0 | ((next_ch%2)<<3) | (next_ch/2)), 0x80};
    uint32_t value; // corresponds to a previously initiated conversion (start_ltc2499_temp)
    i2c_writeread(i2c,0x15,cmd,2,(uint8_t*)&value,4);
    double volts = ((value>>6) & 0x1FFFFFF)*1.25/pow(2,24);
    return volts > 1.25 ? volts-2*1.25 : volts;
}

double read_ad7414_temp(i2c_t *i2c, uint8_t slave) {
    uint8_t read_cmd[2] = { 0x00, 0x00 };
    i2c_write(i2c,slave,read_cmd,2);
    uint8_t val[2] = {0xCA,0xFE};
    i2c_read(i2c,slave,val,2);
    uint16_t temp = (val[0]<<2) | ((val[1]>>6)&0x3);
    if (temp & 0x200) {
        return (temp-512.0)/4.0;
    } else {
        return temp/4.0;
    }
}

void enable_ltc2990(i2c_t *i2c, uint8_t slave) {
   i2c_reg_write(i2c,slave,0x01,0x1F);
   i2c_reg_write(i2c,slave,0x02,0xFF);
   usleep(10000);
}

//ch 1 to ch 4, 5 == Vcc
uint16_t read_ltc2990_value(i2c_t *i2c, uint8_t slave, uint8_t ch) {
    uint8_t reg = (uint8_t)(6+(ch-1)*2);
    uint16_t value;
    i2c_block_read(i2c,slave,reg,(uint8_t*)&value,2);
    return value&0x3FFF;
    
}

void enable_ltc2991(i2c_t *i2c, uint8_t slave) {
   i2c_reg_write(i2c,slave,0x01,0xF8);
   usleep(10000);
}

//ch 1 to ch 8, 9 == T_internal, 10 == Vcc
uint16_t read_ltc2991_value(i2c_t *i2c, uint8_t slave, uint8_t ch) {
    uint8_t reg = (uint8_t)(0xA+(ch-1)*2);
    uint16_t value;
    i2c_block_read(i2c,slave,reg,(uint8_t*)&value,2);
    return value&0x3FFF;
    
}
