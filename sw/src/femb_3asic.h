#ifndef femb_3asic_h
#define femb_3asic_h

#include <cstddef>
#include "io_reg.h"
#include "log.h"

// Fast Command AXI registers
constexpr uint32_t REG_FAST_CMD_CODE       = 0x0000/4;
constexpr uint32_t REG_FAST_CMD_ACT_DELAY  = 0x0004/4;

// COLDATA I2C AXI registers
constexpr uint32_t REG_COLD_I2C_START      = 0x0000/4;
constexpr uint32_t REG_COLD_I2C_CTRL       = 0x0004/4;

// Fast Command bits
constexpr uint8_t FAST_CMD_RESET    = 1;
constexpr uint8_t FAST_CMD_ACT      = 2;
constexpr uint8_t FAST_CMD_SYNC     = 4;
constexpr uint8_t FAST_CMD_EDGE     = 8;
constexpr uint8_t FAST_CMD_IDLE     = 16;
constexpr uint8_t FAST_CMD_EDGE_ACT = 32;

// Fast Command Act commands
constexpr uint8_t ACT_IDLE              = 0x00;
constexpr uint8_t ACT_LARASIC_PULSE     = 0x01;
constexpr uint8_t ACT_SAVE_TIME         = 0x02;
constexpr uint8_t ACT_SAVE_STATUS       = 0x03;
constexpr uint8_t ACT_CLEAR_SAVE        = 0x04;
constexpr uint8_t ACT_RESET_COLDADC     = 0x05;
constexpr uint8_t ACT_RESET_LARASIC     = 0x06;
constexpr uint8_t ACT_RESET_LARASIC_SPI = 0x07;
constexpr uint8_t ACT_PROGRAM_LARASIC   = 0x08;

// COLDATA I2C control bit packing scheme
constexpr uint32_t COLD_I2C_CHIP_ADDR   = 23; //4 // 0x07800000;
constexpr uint32_t COLD_I2C_REG_PAGE    = 20; //3 // 0x00700000;
constexpr uint32_t COLD_I2C_RW          = 19; //1 // 0x00080000;
constexpr uint32_t COLD_I2C_ACK1        = 18; //1 // 0x00040000;
constexpr uint32_t COLD_I2C_REG_ADDR    = 10; //8 // 0x0003FC00;
constexpr uint32_t COLD_I2C_ACK2        =  9; //1 // 0x00000200;
constexpr uint32_t COLD_I2C_DATA        =  1; //8 // 0x000001FE;
constexpr uint32_t COLD_I2C_ACK3        =  0; //1 // 0x00000001;

// COLDATA I2C delay
constexpr uint32_t COLD_I2C_DELAY = 60; //microseconds

// COLDATA Chip Addresses (unused...)
constexpr uint8_t CHIP_CD_BOT       = 0x2;
constexpr uint8_t CHIP_CD_TOP       = 0x3;
constexpr uint8_t CHIP_CD_BOT_ADC0  = 0x4;
constexpr uint8_t CHIP_CD_BOT_ADC1  = 0x5;
constexpr uint8_t CHIP_CD_BOT_ADC2  = 0x6;
constexpr uint8_t CHIP_CD_BOT_ADC3  = 0x7;
constexpr uint8_t CHIP_CD_TOP_ADC0  = 0x8;
constexpr uint8_t CHIP_CD_TOP_ADC1  = 0x9;
constexpr uint8_t CHIP_CD_TOP_ADC2  = 0xa;
constexpr uint8_t CHIP_CD_TOP_ADC3  = 0xb;

enum FrameType {
    FRAME_DD,
    FRAME_12,
    FRAME_14
};

//LArASIC configuration (assumes all channels the same)
typedef struct {
    bool sdd,sdc,slkh,s16,stb,stb1,slk,sgp; //glog.reg 1
    uint8_t sdac; bool sdacsw1,sdacsw2; // glog.reg 2
    bool sts,snc; uint8_t gain, peak_time; bool smn,sdf; // chan regs
    uint8_t cal_skip, cal_delay, cal_length; // COLDATA registers controlling calibration strobe
} larasic_conf;

typedef struct {
    uint8_t reg_0, reg_4;
    uint8_t reg_24, reg_25, reg_26, reg_27;
    uint8_t reg_29, reg_30;
} coldadc_conf;

class FEMB_3ASIC {

public:
    // Initialize a FEMB index [0-3]
    FEMB_3ASIC(int index);
    ~FEMB_3ASIC();
    
    // Front end I2C configuration
    bool configure_coldata(bool cold, FrameType frame);
    bool configure_coldadc(bool cold, bool test_pattern = false, coldadc_conf *conf = NULL);
    bool configure_larasic(const larasic_conf &c);
    bool set_fast_act(uint8_t act_cmd);
    bool read_spi_status(); // requires ACT_SAVE_STATUS first
    void log_spi_status(); // requires ACT_SAVE_STATUS first
    
    // for manual calibration 
    bool setup_calib(uint8_t sn, uint8_t stage);
    bool store_calib(const uint16_t w0_vals[8][2], const uint16_t w2_vals[8][2], uint8_t stage);
    
    // for setting the FMB_CONTROL_WORD
    bool set_control_reg(uint8_t coldata_idx, bool ctrl_0, bool ctrl_1);
    
    // Send a fast command to all FEMBs
    static void fast_cmd(uint8_t cmd_code);
    
    // Read/Write to the specified COLDATA's I2C on this FEMB
    void i2c_write(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    uint8_t i2c_read(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page,  uint8_t reg_addr);
    // Perform a Write and a Read, returning read == data
    bool i2c_write_verify(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data, size_t retries = 30);

protected:

    // The index'th FEMB connected to this WIB
    int index;
    
    // The last chip address accessed via each I2C bus
    int last_coldata_i2c_chip[2];
    
    // Interface to each COLDATA's I2C firmware controller
    io_reg_t coldata_i2c[2];
    
    // Fast command firmware interface for all FEMBs
    static io_reg_t coldata_fast_cmd;
    
    // Called internally before any I2C access to ensure the chip address is correctly latched
    void i2c_bugfix(uint8_t bus_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    
};

#endif
