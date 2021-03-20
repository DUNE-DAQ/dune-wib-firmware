#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "io_reg.h"

#ifndef SIMULATION

int io_reg_init(io_reg_t *reg, size_t base_addr, size_t n_reg) {

	reg->base_addr = base_addr;
	reg->fd=open("/dev/mem",O_RDWR|O_SYNC);
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

void io_reg_write(io_reg_t *reg, size_t idx, uint32_t data, uint32_t mask) {
    if (mask == 0xFFFFFFFF) {
        reg->ptr[idx] = data;
    } else {
        uint32_t prev = io_reg_read(reg,idx);
        data = (data & mask) | ((~mask) & prev);
        reg->ptr[idx] = data;
    }
}

#else

#include <zmq.hpp>

#define READ 0
#define WRITE 1
#define DONE 2


typedef struct {
    unsigned int cmd, addr, data;
} msg;
    
zmq::context_t axi_server_context;
zmq::socket_t axi_server_socket(axi_server_context, ZMQ_PAIR);
bool axi_server_init = false;

void init_axi_server_socket() {
    if (axi_server_init) return;
    glog.log("Connecting to cocotb axi_server...\n"); 
    axi_server_socket.connect("tcp://127.0.0.1:7777");
    axi_server_init = true;
}

unsigned int axi_server(unsigned int cmd, unsigned int addr = 0, unsigned int data = 0) {
    init_axi_server_socket();
    zmq::message_t zm(sizeof(msg));
    msg *m = (msg*)zm.data();
    m->cmd = cmd;
    m->addr = addr;
    m->data = data;
  
    //glog.log("Skipping AXI simulation!\n");
    //axi_server_socket.send(zm,zmq::send_flags::none);
    //axi_server_socket.recv(zm,zmq::recv_flags::none);
    
    m = (msg*) zm.data();
    return m->data;
}

int io_reg_init(io_reg_t *reg, size_t base_addr, size_t n_reg) {
    reg->base_addr = base_addr;
	return 0;
}

int io_reg_free(io_reg_t *reg) {
    return 0;
}

uint32_t io_reg_read(io_reg_t *reg, size_t idx) {
    glog.log("axi_read %lx\n",idx*4+reg->base_addr);
    return axi_server(READ,idx*4+reg->base_addr);
}

void io_reg_write(io_reg_t *reg, size_t idx, uint32_t data, uint32_t mask) {
    if (mask == 0xFFFFFFFF) {
        glog.log("axi_write %lx = %08X\n",idx*4+reg->base_addr,data);
        axi_server(WRITE,idx*4+reg->base_addr,data);
    } else {
        uint32_t prev = io_reg_read(reg,idx);
        data = (data & mask) | ((~mask) & prev);
        glog.log("axi_write %lx = %08X mask %08X\n",idx*4+reg->base_addr,data,mask);
        axi_server(WRITE,idx*4+reg->base_addr,data);
    }
}

#endif

