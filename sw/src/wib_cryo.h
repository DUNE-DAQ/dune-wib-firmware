#ifndef wib_cryo_h
#define wib_cryo_h

#include "wib.h"
#include "femb_cryo.h"

class WIB_CRYO : public WIB {

public:

    WIB_CRYO();
    virtual ~WIB_CRYO();

    // Turns calibration pulser on or off 
    virtual bool set_pulser(bool on);
    
    // Control power sequence for this WIB's frontend 
    virtual bool power_wib(wib::PowerWIB &conf);
    
    // Configure the frontend for this WIB
    virtual bool configure_wib(wib::ConfigureWIB &conf);
    
protected:

    // Interface to each of the 4 FEMBs 
    FEMB_CRYO* femb[4];
    
};

#endif
