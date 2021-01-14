#ifndef wib_h
#define wib_h

#include "wib.pb.h"

#include "femb.h"
#include "i2c.h"
#include "io_reg.h"
#include "log.h"

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

//Memory base addresses of AXI interfaces
#define CTRL_REGS           0xA00C0000
#define DAQ_SPY_0           0xA0100000
#define DAQ_SPY_1           0xA0200000
#define DAQ_SPY_SIZE        0x00100000

//32bit register index in CTRL_REGS
#define REG_TIMING          0x0000/4
#define REG_FW_CTRL         0x0004/4
#define REG_LINK_MASK       0x0008/4
#define REG_FAKE_TIME_CTRL  0x000C/4
#define REG_FAKE_TIME_L     0x0018/4
#define REG_FAKE_TIME_H     0x001C/4
#define REG_DAQ_SPY_STATUS  0x0080/4
#define REG_FW_TIMESTAMP    0x0088/4

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
    
    // Reads determines IP from backplane, else returns default 192.168.8.1
    std::string crate_ip();
    // Reads determines IP for the default gateway, else returns default 192.168.8.254
    std::string gateway_ip();
    
    // Reset and configure the timing endpoint
    bool timing_endpoint_config();
    
    // Sets the default voltage configurations for FEMB power controller
    bool femb_power_config();
    // Set FEMB power on or off per FEMB (first colddata then coldadc)
    bool femb_power_set(int femb_idx, bool on);
    // Set the FEMB serial receiver mask value
    bool femb_rx_mask(uint32_t value, uint32_t mask = 0xFFFF);
    // Resets the FEMB serial receivers
    bool femb_rx_reset();
    // Adjust FEMB power regulator settings (does not enable or disable)
    bool femb_power_reg_ctrl(uint8_t femb_id, uint8_t regulator_id, double voltage);
    
    // Calls the `reboot` system command
    bool reboot();
    // Calls the wib_update.sh script after unpacking the two tar archives
    bool update(const std::string &root_archive, const std::string &boot_archive);
    
    // Instructs firmware to attach a specific I2C device to the selected_i2c interface
    // See I2C_* defines in this file for options
    void i2c_select(uint8_t device);
    
    // Instructs firmware to fill the daq spy buffers and copies them into the argument pointers
    // Preallocate DAQ_SPY_SIZE buffers, or pass NULL to ignore that buffer
    bool read_daq_spy(void *buf0, void *buf1);
    
    // Read/Write the WIB address space (AXI,etc)
    uint32_t peek(size_t addr);
    void poke(size_t addr, uint32_t value);
    
    // Read/Write the COLDATA i2c address space for a particular (femb[0-3],coldata[0-1]) index 
    uint8_t cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr);
    void cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data);
    
    // Turns pulser on or off  
    bool set_pulser(bool on);
    
    // Control power sequence for frontend 
    bool power_wib(wib::PowerWIB &conf);
    
    // Configure this WIB
    bool configure_wib(wib::ConfigureWIB &conf);

    // Read the onboard I2C sensors and fill the sensor structure
    bool read_sensors(wib::GetSensors::Sensors &sensors);
    
    // Read the firmware timestamp code
    uint32_t read_fw_timestamp();
    
    // Set fake time generator start value and halt generator
    // This does not start timestamp increment, call start_fake_time for that
    void set_fake_time(uint64_t time);
    // Start fake time generator using previously set start time
    void start_fake_time();
    
    
protected:
    
    // Front end initialized
    bool frontend_initialized = false;
    
    // Save the FEMB power state (could perhaps read from i2c)
    bool frontend_power[4];
    
    // Save the pulser state (no way to read from hardware)
    bool pulser_on;
    
    // Interface to each of the 4 FEMBs 
    FEMB* femb[4];
    
    // I2C interface to the selectable I2C bus (see i2c_select)
    i2c_t selected_i2c;
    
    // I2C interface to the I2C bus with FEMB power enable
    i2c_t femb_en_i2c;
    
    // I2C interface to the I2C bus with FEMB power monitor
    i2c_t femb_pwr_i2c;
    
    // AXI interface to firmware control and status registers 
    io_reg_t regs;

    // File descriptor for the daq spy mapped memory
    int daq_spy_fd;
    // Pointers to the daq spy firmware buffers
    void *daq_spy[2];
    
    // Power on and reset the frontend (called on first configure_wib)
    bool start_frontend();

};

#endif
