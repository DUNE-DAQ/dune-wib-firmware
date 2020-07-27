#ifndef wib_h
#define wib_h

#include "wib.pb.h"

#include "femb.h"
#include "i2c.h"
#include "io_reg.h"

class WIB {

public:
    WIB();
    ~WIB();
    
    bool initialize();
    bool read_sensors(wib::Sensors &sensors);
    
protected:

    FEMB* femb[4];
    i2c_t i2c;
    io_reg_t regs;

};

#endif
