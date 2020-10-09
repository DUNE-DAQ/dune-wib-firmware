#ifndef wib_h
#define wib_h

#include "wib.pb.h"

#include "femb.h"
#include "i2c.h"
#include "io_reg.h"

#include <cstdint>

class WIB {

public:
    WIB();
    ~WIB();
    
    bool initialize();
    bool reboot();
    bool update(const std::string &root_archive, const std::string &boot_archive);
    
    //Read/Write the WIB address space
    uint32_t peek(size_t addr);
    void poke(size_t addr, uint32_t value);
    
    //Read/Write the COLDATA i2c address space for a particular (femb,coldata) index 
    uint8_t cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    void cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    
    bool read_sensors(wib::Sensors &sensors);
    
protected:

    FEMB* femb[4];
    i2c_t i2c;
    io_reg_t regs;
    io_reg_t leds;

};

#endif
