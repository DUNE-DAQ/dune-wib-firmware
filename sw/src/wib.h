#ifndef wib_h
#define wib_h

#include "wib.pb.h"

#include "femb.h"
#include "i2c.h"
#include "io_reg.h"

#include <cstdint>

#define I2C_SI5344          0
#define I2C_SI5342          1
#define I2C_QSFP            2
#define I2C_PL_FEMB_PWR     3
#define I2C_PL_FEMB_EN      4
#define I2C_SENSOR          5
#define I2C_PL_FEMB_PWR2    6
#define I2C_LTC2977         7
#define I2C_PL_FEMB_PWR3    8
#define I2C_FLASH           9
#define I2C_ADN2814         10

#define CTRL_REGS           0xA00C0000
#define DAQ_SPY_0           0xA0100000
#define DAQ_SPY_1           0xA0200000
#define DAQ_SPY_SIZE        0x00100000

class WIB {

public:
    WIB();
    ~WIB();
    
    
    bool script_cmd(std::string line);
    bool script(std::string script, bool file = true);
    
    bool initialize();
    bool set_ip(std::string ip);
    
    bool femb_power_config();
    bool femb_power_set(bool on);
    bool femb_serial_reset();
    bool femb_power_ctrl(uint8_t femb_id, uint8_t regulator_id, double voltage);
    
    bool reboot();
    bool update(const std::string &root_archive, const std::string &boot_archive);
    
    void i2c_select(uint8_t device);
    
    void read_daq_spy(void *buf0, void *buf1);
    
    //Read/Write the WIB address space
    uint32_t peek(size_t addr);
    void poke(size_t addr, uint32_t value);
    
    //Read/Write the COLDATA i2c address space for a particular (femb,coldata) index 
    uint8_t cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    void cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    
    bool read_sensors(wib::Sensors &sensors);
    
protected:
    
    FEMB* femb[4];
    
    i2c_t selected_i2c;
    i2c_t power_i2c;
    io_reg_t regs;

    int daq_spy_fd;
    void *daq_spy[2];

};

#endif
