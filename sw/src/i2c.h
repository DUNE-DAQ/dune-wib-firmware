#ifndef i2c_h
#define i2c_h

#include <stdint.h>

typedef struct i2c_t {
	int fd;
	uint8_t slave;
} i2c_t;

int i2c_init(i2c_t *i2c, char *device);

int i2c_free(i2c_t *i2c);

int i2c_read(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len);

int i2c_write(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len);

#endif

