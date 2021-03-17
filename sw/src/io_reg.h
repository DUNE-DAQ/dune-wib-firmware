#ifndef io_reg_h
#define io_reg_h

#include <stdint.h>
#include "log.h"

typedef struct io_reg_t {
	int fd;
	uint32_t *ptr;
	size_t n_reg;
	size_t base_addr;
} io_reg_t;

int io_reg_init(io_reg_t *reg, size_t base_addr, size_t n_reg);

int io_reg_free(io_reg_t *reg);

uint32_t io_reg_read(io_reg_t *reg, size_t idx);

void io_reg_write(io_reg_t *reg, size_t idx, uint32_t data, uint32_t mask = 0xFFFFFFFF);

#endif

