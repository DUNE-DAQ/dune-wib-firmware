#ifndef wib_3asic_h
#define wib_3asic_h

#include "wib.h"
#include "femb_3asic.h"

//32bit register index in CTRL_REGS
constexpr size_t REG_LINK_MASK       = 0x0008/4;

class WIB_3ASIC : public WIB {

public:

    WIB_3ASIC();
    virtual ~WIB_3ASIC();

    // Read/Write the COLDATA i2c address space for a particular (femb[0-3],coldata[0-1]) index 
    uint8_t cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    void cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    
    // Turns calibration pulser on or off 
    virtual bool set_pulser(bool on);
    
    // Control power sequence for this WIB's frontend 
    virtual bool power_wib(const wib::PowerWIB &conf);
    
    // Configure the frontend for this WIB
    virtual bool configure_wib(const wib::ConfigureWIB &conf);
    
    // Calibrate the COLDADCs manually
    virtual bool calibrate();
    
protected:

    // Interface to each of the 4 FEMBs 
    FEMB_3ASIC* femb[4];
    
    // Front end initialized
    bool frontend_initialized = false;
    
    // Save the pulser state (no way to read from hardware)
    bool pulser_on = false;    
    
    // Handles `fast` and `cd-i2c` script commands
    virtual bool script_extra(const std::vector<std::string> &tokens);
    
    // Set FEMB power on or off per FEMB
    // May subclass and override this as ASICs/FEMBs undergo revisions
    virtual bool femb_power_set(int femb_idx, bool on, bool cold = true);
    
    // Set the coldata serial receiver mask value
    bool femb_rx_mask(uint32_t value, uint32_t mask = 0xFFFF);
    
    // Resets the coldata serial receivers
    bool femb_rx_reset();
    
    // Reset front end to a powered off state
    virtual bool reset_frontend();
};

#endif
