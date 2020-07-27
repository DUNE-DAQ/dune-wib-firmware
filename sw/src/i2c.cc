#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "i2c.h"

#ifndef SIMULATION

int i2c_init(i2c_t *i2c, char *device) {
	i2c->fd = open(device,O_RDWR);
	if (i2c->fd < 1) return 1;
	i2c->slave = 0;
	return 0;
}

int i2c_free(i2c_t *i2c) {
    close(i2c->fd);
    return 0;
}

int i2c_read(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len) {
    if (slave != i2c->slave) {
        ioctl(i2c->fd, I2C_SLAVE, slave);
    }
    return read(i2c->fd,buf,len) != len;
}

int i2c_write(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len) {
    if (slave != i2c->slave) {
        ioctl(i2c->fd, I2C_SLAVE, slave);
    }
    return write(i2c->fd,buf,len) != len;
}

#else

int i2c_init(i2c_t *i2c, char *device) {
	return 0;
}

int i2c_free(i2c_t *i2c) {
    return 0;
}

int i2c_read(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len) {
    return 0;
}

int i2c_write(i2c_t *i2c, uint8_t slave, uint8_t *buf, size_t len) {
    return 0;
}

#endif

