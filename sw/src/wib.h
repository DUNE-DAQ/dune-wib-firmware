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
    
    // Process a single line from a script. See github readme for script format.
    bool script_cmd(std::string line);
    
    // Execute a script. Either a filename to be opened (file = true) or a script lines (file = false)
    bool script(std::string script, bool file = true);
    
    // Startup initialization
    bool initialize();
    
    // Reads determins IP from backplane, else returns default 192.168.8.1
    std::string crate_ip();
    
    // Sets the default voltage configurations for FEMB power controller
    bool femb_power_config();
    // Turns femb power on or off (all at once)
    bool femb_power_set(bool on);
    // Resets the FEMB serial transmitters
    bool femb_serial_reset();
    // Adjust FEMB power settings (does not enable or disable)
    bool femb_power_ctrl(uint8_t femb_id, uint8_t regulator_id, double voltage);
    
    // Calls the `reboot` system command
    bool reboot();
    // Calls the wib_update.sh script after unpacking the two tar archives
    bool update(const std::string &root_archive, const std::string &boot_archive);
    
    // Instructs firmware to attach a specific I2C device to the selected_i2c interface
    // See I2C_* defines in this fiel for options
    void i2c_select(uint8_t device);
    
    // Instructs firmware to fill the daq spy buffers and copies them into the argument pointers
    // Preallocate DAQ_SPY_SIZE buffers, or pass NULL to ignore that buffer
    bool read_daq_spy(void *buf0, void *buf1);
    
    // Read/Write the WIB address space (AXI,etc)
    uint32_t peek(size_t addr);
    void poke(size_t addr, uint32_t value);
    
    // Read/Write the COLDATA i2c address space for a particular (femb,coldata) index 
    uint8_t cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    void cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    
    // Configure this WIB
    bool configure_wib(wib::ConfigureWIB &conf);
    
    // Configure a FEMB
    bool configure_femb(wib::ConfigureFEMB &conf);
    
    // Read the onboard I2C sensors and fill the sensor structure
    bool read_sensors(wib::Sensors &sensors);
    
protected:
    
    // Interface to each of the 4 FEMBs 
    FEMB* femb[4];
    
    // I2C interface to the selectable I2C bus (see i2c_select)
    i2c_t selected_i2c;
    
    // I2C interface to the I2C bus with FEMB power control
    i2c_t power_i2c;
    
    // AXI interface to firmware control and status registers 
    io_reg_t regs;

    // File descriptor for the daq spy mapped memory
    int daq_spy_fd;
    // Pointers to the daq spy firmware buffers
    void *daq_spy[2];

};

#endif
