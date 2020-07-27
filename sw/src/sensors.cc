#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "sensors.h"
#include "i2c.h"
#include "io_reg.h"

double read_ltc2499_temp(i2c_t *i2c, uint8_t ch) {
    uint8_t cmd[2] = { 0xB0 | ((ch%2)<<3) | (ch/2), 0x80};
    i2c_write(i2c,0x15,cmd,2);
    usleep(300000);
    uint32_t value;
    i2c_read(i2c,0x15,(uint8_t*)&value,4);
    double volts = ((value>>6) & 0xFFFFFF)*1.25/pow(2,23);
    return volts > 1.25 ? volts-2*1.25 : volts;
}

uint8_t read_ad7414_temp(i2c_t *i2c, uint8_t slave) {
    uint8_t read_cmd[1] = { 0x00 };
    i2c_write(i2c,slave,read_cmd,2);
    uint8_t val;
    i2c_read(i2c,slave,&val,1);
    return val;
}

void enable_ltc2990(i2c_t *i2c, uint8_t slave) {
   uint8_t enable_cmd[2] = { 0x01,0x1F };
   i2c_write(i2c,slave,enable_cmd,2);
   uint8_t measure_cmd[2] = { 0x02,0xFF };
   i2c_write(i2c,slave,measure_cmd,2);
   usleep(10000);
}

//ch 1 to ch 4, Vcc
uint16_t read_ltc2990_value(i2c_t *i2c, uint8_t slave, uint8_t ch) {
    uint8_t read_cmd[1] = { 6+(ch-1)*2 };
    i2c_write(i2c,slave,read_cmd,1);
    uint16_t value;
    i2c_read(i2c,slave,(uint8_t*)&value,2);
    return value&0x3FFF;
    
}

void enable_ltc2991(i2c_t *i2c, uint8_t slave) {
   uint8_t en_meas_cmd[2] = { 0x01,0xF8 };
   i2c_write(i2c,slave,en_meas_cmd,2);
   usleep(10000);
}

//ch 1 to ch 4, T_internal, Vcc
uint16_t read_ltc2991_value(i2c_t *i2c, uint8_t slave, uint8_t ch) {
    uint8_t read_cmd[1] = { 0xA+(ch-1)*2 };
    i2c_write(i2c,slave,read_cmd,1);
    uint16_t value;
    i2c_read(i2c,slave,(uint8_t*)&value,2);
    return value&0x3FFF;
    
}
