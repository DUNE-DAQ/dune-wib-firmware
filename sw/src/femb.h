#ifndef femb_h
#define femb_h

#include <cstddef>
#include "io_reg.h"

// Fast Command AXI registers
constexpr uint32_t REG_FAST_CMD_CODE        = 0/4;
constexpr uint32_t REG_FAST_CMD_ACT_DELAY   = 4/4;

// COLDATA I2C AXI registers
constexpr uint32_t REG_COLD_I2C_START        = 0/4;
constexpr uint32_t REG_COLD_I2C_CTRL         = 4/4;

// Fast Command bits
constexpr uint8_t FAST_CMD_RESET = 1;
constexpr uint8_t FAST_CMD_ACT = 2;
constexpr uint8_t FAST_CMD_SYNC = 4;
constexpr uint8_t FAST_CMD_EDGE = 8;
constexpr uint8_t FAST_CMD_IDLE = 16;
constexpr uint8_t FAST_CMD_EDGE_ACT = 32;

// COLDATA I2C control bit packing scheme
constexpr uint32_t COLD_I2C_CHIP_ADDR   = 23; //4 //0x07800000;
constexpr uint32_t COLD_I2C_REG_PAGE    = 20; //3 // 0x00700000;
constexpr uint32_t COLD_I2C_RW          = 19; //1 // 0x00080000;
constexpr uint32_t COLD_I2C_ACK1        = 18; //1 // 0x00040000;
constexpr uint32_t COLD_I2C_REG_ADDR    = 10; //8 // 0x0003FC00;
constexpr uint32_t COLD_I2C_ACK2        = 9; //1 // 0x00000200;
constexpr uint32_t COLD_I2C_DATA        = 1; //8 // 0x000001FE;
constexpr uint32_t COLD_I2C_ACK3        = 0; //1 // 0x00000001;

class FEMB {

public:
    // Initialize a FEMB index [0-3]
    FEMB(int index);
    ~FEMB();
    
    // Send a fast command to all FEMBs
    static void fast_cmd(uint8_t cmd_code);
    
    // Read/Write to the specified COLDATA's I2C on this FEMB
    void i2c_write(uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    uint8_t i2c_read(uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page,  uint8_t reg_addr);

protected:
    
    // The last chip address accessed via each I2C bus
    int last_coldata_i2c_chip[2];
    
    // Interface to each COLDATA's I2C firmware controller
    io_reg_t coldata_i2c[2];
    
    // Fast command firmware interface for all FEMBs
    static io_reg_t coldata_fast_cmd;
    
    // Called internally before any I2C access to ensure the chip address is correctly latched
    void i2c_bugfix(uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    
};

#endif
