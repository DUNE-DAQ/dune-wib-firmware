#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "io_reg.h"

#ifndef SIMULATION

int io_reg_init(io_reg_t *reg, size_t base_addr, size_t n_reg) {
	
	reg->fd=open("/dev/mem",O_RDWR);
	if (reg->fd < 1) return 1;

	size_t ptr = (size_t) mmap(NULL,n_reg*4,PROT_READ|PROT_WRITE,MAP_SHARED,reg->fd,base_addr);
	if (ptr == -1) return 2;

    reg->ptr = (uint32_t*)(ptr);
    reg->n_reg = n_reg;
    return 0;
}

int io_reg_free(io_reg_t *reg) {
    munmap(reg->ptr,reg->n_reg*4);
    close(reg->fd);
    return 0;
}

uint32_t io_reg_read(io_reg_t *reg, size_t idx) {
    return reg->ptr[idx];
}

void io_reg_write(io_reg_t *reg, size_t idx, uint32_t data) {
    reg->ptr[idx] = data;
}

#else

int io_reg_init(io_reg_t *reg, size_t base_addr, size_t n_reg) {
	return 0;
}

int io_reg_free(io_reg_t *reg) {
    return 0;
}

uint32_t io_reg_read(io_reg_t *reg, size_t idx) {
    return 0;
}

void io_reg_write(io_reg_t *reg, size_t idx, uint32_t data) {
    return;
}

#endif

