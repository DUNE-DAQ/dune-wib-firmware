#ifndef sensors_h
#define sensors_h

#include "i2c.h"
#include "log.h"

void start_ltc2499_temp(i2c_t *i2c, uint8_t ch);
double read_ltc2499_temp(i2c_t *i2c, uint8_t ch);

double read_ad7414_temp(i2c_t *i2c, uint8_t slave);

void enable_ltc2990(i2c_t *i2c, uint8_t slave, bool differential = false);

//ch 1 to ch 4, Vcc
uint16_t read_ltc2990_value(i2c_t *i2c, uint8_t slave, uint8_t ch);

void enable_ltc2991(i2c_t *i2c, uint8_t slave, bool differential = false);

//ch 1 to ch 4, T_internal, Vcc
uint16_t read_ltc2991_value(i2c_t *i2c, uint8_t slave, uint8_t ch);

#endif
