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

int read_sensors(wib::Sensors &sensors) {

   io_reg_t reg;
   io_reg_init(&reg,0xA0002000,32);
   
   printf("Activating I2C_SENSOR bus\n");
   io_reg_write(&reg,1,0x00000005); //reg1 = 0x5
   
   i2c_t i2c;
   i2c_init(&i2c,"/dev/i2c-0");
   
   printf("Enabling voltage sensors\n");
   uint8_t buf[1] = {0x7};
   i2c_write(&i2c,0x70,buf,1); //
   // 0x48 LTC2991
   // 0x4C 0x4E  LTC2990
   
   enable_ltc2990(&i2c,0x4E);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4E ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&i2c,0x4E,i));
   }
   printf("LTC2990 0x4E Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&i2c,0x4E,5)+2.5);
   
   enable_ltc2990(&i2c,0x4C);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4C ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&i2c,0x4C,i));
   }
   printf("LTC2990 0x4C Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&i2c,0x4C,5)+2.5);
   
   enable_ltc2991(&i2c,0x48);
   for (uint8_t i = 1; i < 8; i++) {
       printf("LTC2991 0x48 ch%i -> %0.2f V\n",i,0.00030518*read_ltc2991_value(&i2c,0x48,i));
   }
   printf("LTC2991 0x48 T -> %0.2f C\n",0.0625*read_ltc2991_value(&i2c,0x48,8));
   printf("LTC2991 0x48 Vcc -> %0.2f V\n",0.00030518*read_ltc2991_value(&i2c,0x48,9)+2.5);
   
   // 0x49 0x4D 0x4A supposedly are AD7414
   printf("AD7414 0x49 temp %i\n", read_ad7414_temp(&i2c,0x49));
   printf("AD7414 0x4D temp %i\n", read_ad7414_temp(&i2c,0x4D));
   printf("AD7414 0x4A temp %i\n", read_ad7414_temp(&i2c,0x4A));
   
   // 0x15 LTC2499
   printf("Reading temperature sensors\n");
   for (uint8_t i = 0; i < 7; i++) {
       printf("LTC2499 ch%i -> %0.14f\n",i,read_ltc2499_temp(&i2c,i));
   }
   
   return 0;
}
