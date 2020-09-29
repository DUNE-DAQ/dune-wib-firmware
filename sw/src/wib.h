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
    
    uint32_t peek(size_t addr);
    uint32_t poke(size_t addr, uint32_t value);
    
    bool read_sensors(wib::Sensors &sensors);
    
protected:

    FEMB* femb[4];
    i2c_t i2c;
    io_reg_t regs;
    io_reg_t leds;

};

#endif
