#include "wib.h"
#include "sensors.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

using namespace std;

WIB::WIB() {
    io_reg_init(&this->regs,CTRL_REGS,0x10000/4);
    i2c_init(&this->selected_i2c,(char*)"/dev/i2c-0");
    i2c_init(&this->power_i2c,(char*)"/dev/i2c-2");
    #ifdef SIMULATION
    this->daq_spy_fd = -1;
    this->daq_spy[0] = new char[DAQ_SPY_SIZE];
    this->daq_spy[1] = new char[DAQ_SPY_SIZE];
    for (size_t i = 0; i < DAQ_SPY_SIZE; i++) {
        ((char*)this->daq_spy[0])[i] = 'A';
        ((char*)this->daq_spy[1])[i] = 'B';
    }
    #else
    this->daq_spy_fd = open("/dev/mem",O_RDWR);
    this->daq_spy[0] = mmap(NULL,DAQ_SPY_SIZE,PROT_READ,MAP_SHARED,this->daq_spy_fd,DAQ_SPY_0);
    this->daq_spy[1] = mmap(NULL,DAQ_SPY_SIZE,PROT_READ,MAP_SHARED,this->daq_spy_fd,DAQ_SPY_1);
    #endif
    for (int i = 0; i < 4; i++) {
        this->femb[i] = new FEMB(i);
    }
}

WIB::~WIB() {
    io_reg_free(&this->regs);
    i2c_free(&this->selected_i2c);
    #ifdef SIMULATION
    delete [] (char*)this->daq_spy[0];
    delete [] (char*)this->daq_spy[1];
    #else
    munmap(this->daq_spy[0],DAQ_SPY_SIZE);
    munmap(this->daq_spy[1],DAQ_SPY_SIZE);
    close(this->daq_spy_fd);
    #endif
    for (int i = 0; i < 4; i++) {
        delete this->femb[i];
    }
}

bool WIB::initialize() {
    bool success = true;
    int ret;
    ret = system("ip link set eth0 up");
    if (WEXITSTATUS(ret) != 0) {
        fprintf(stderr,"failed to bring up eth0\n");
        success = false;
    }
    string eth0_conf("ip link addr add "+crate_ip()+" dev eth0");
    ret = system(eth0_conf.c_str());
    if (WEXITSTATUS(ret) != 0) {
        fprintf(stderr,"failed to assign IP to eth0\n");
        success = false;
    }
    success &= script("startup"); //FIXME this should be the correct initial setup
    return success;
}

string WIB::crate_ip() {
    printf("FIXME: using default IP: 192.168.8.1/24\n");
    return "192.168.8.1/24"; //FIXME pull from firmware
}

bool WIB::femb_power_ctrl(uint8_t femb_id, uint8_t regulator_id, double voltage) {
    uint8_t chip;
    uint8_t reg;
    uint8_t buffer[2];
    uint32_t DAC_value;

    switch (regulator_id) {
        case 0:
        case 1:
        case 2:
        case 3:
            i2c_select(I2C_PL_FEMB_PWR2);   // SET I2C mux to 0x06 for FEMB DC2DC DAC access
            DAC_value   = (uint32_t) ((voltage * -482.47267) + 2407.15);
            reg         = (uint8_t) (0x10 | ((regulator_id & 0x0f) << 1));
            buffer[0]   = (uint8_t) (DAC_value >> 4) & 0xff;
            buffer[1]   = (uint8_t) (DAC_value << 4) & 0xf0;
            switch(femb_id) {
                case 0:
                    chip = 0x4C;
                    break;  
                case 1:
                    chip = 0x4D;
                    break;  
                case 2:
                    chip = 0x4E;
                    break;  
                case 3:
                    chip = 0x4F;
                    break;  
                default:
                    return false;
            }
            break;
        case 4:
            i2c_select(I2C_PL_FEMB_PWR3);   // SET I2C mux to 0x08 for FEMB LDO DAC access
            chip = 0x4C;
            reg  = (0x10 | ((femb_id & 0x0f) << 1));
            DAC_value   = (uint32_t) ((voltage * -819.9871877) + 2705.465);
            buffer[0]   = (uint8_t) (DAC_value >> 4) & 0xff;
            buffer[1]   = (uint8_t) (DAC_value << 4) & 0xf0;
            break;
        case 5:
            i2c_select(I2C_PL_FEMB_PWR3);   // SET I2C mux to 0x08 for FEMB LDO DAC access
            chip = 0x4D;
            reg  = (0x10 | ((femb_id & 0x0f) << 1));
            DAC_value   = (uint32_t) ((voltage * -819.9871877) + 2705.465);
            buffer[0]   = (uint8_t) (DAC_value >> 4) & 0xff;
            buffer[1]   = (uint8_t) (DAC_value << 4) & 0xf0;
            break;
        default:
            return false;
    }

    i2c_block_write(&this->selected_i2c,chip,reg,buffer,2);

    return true;
}

bool WIB::femb_power_config() {
    for (int i = 0; i <= 3; i++) {
        femb_power_ctrl(i, 0, 4.0);
        femb_power_ctrl(i, 1, 4.0);
        femb_power_ctrl(i, 2, 4.0);
        femb_power_ctrl(i, 3, 4.0);
        femb_power_ctrl(i, 4, 2.5);
        femb_power_ctrl(i, 5, 2.5);
    }
    
    return true;
}

bool WIB::femb_power_set(bool on) {
    if (on) {
        // configure all pins as outputs
        i2c_reg_write(&this->power_i2c, 0x22, 0xC, 0);
        i2c_reg_write(&this->power_i2c, 0x22, 0xD, 0);
        i2c_reg_write(&this->power_i2c, 0x22, 0xE, 0);
        // set all ones on all outputs
        i2c_reg_write(&this->power_i2c, 0x22, 0x4, 0xFF);
        i2c_reg_write(&this->power_i2c, 0x22, 0x5, 0xFF);
        i2c_reg_write(&this->power_i2c, 0x22, 0x6, 0xFF);
        // configure all pins as outputs
        i2c_reg_write(&this->power_i2c, 0x23, 0xC, 0);
        i2c_reg_write(&this->power_i2c, 0x23, 0xD, 0);
        i2c_reg_write(&this->power_i2c, 0x23, 0xE, 0);
        // set all ones on all outputs
        i2c_reg_write(&this->power_i2c, 0x23, 0x4, 0xFF);
        i2c_reg_write(&this->power_i2c, 0x23, 0x5, 0xFF);
        i2c_reg_write(&this->power_i2c, 0x23, 0x6, 0xFF);
    } else {
        // set all ones on all outputs
        i2c_reg_write(&this->power_i2c, 0x22, 0x4, 0);
        i2c_reg_write(&this->power_i2c, 0x22, 0x5, 0);
        i2c_reg_write(&this->power_i2c, 0x22, 0x6, 0);
        // set all ones on all outputs
        i2c_reg_write(&this->power_i2c, 0x23, 0x4, 0);
        i2c_reg_write(&this->power_i2c, 0x23, 0x5, 0);
        i2c_reg_write(&this->power_i2c, 0x23, 0x6, 0);
    }
    return true;
}

bool WIB::femb_serial_reset() {
    uint32_t value = io_reg_read(&this->regs,0x04/4);
    value |= (1<<7);
    io_reg_write(&this->regs,0x04/4,value);
    value &= ~(1<<7);
    io_reg_write(&this->regs,0x04/4,value);
    return true;
}

bool WIB::script_cmd(string line) {
    istringstream ss(line);
    istream_iterator<string> begin(ss);
    istream_iterator<string> end;
    vector<string> tokens(begin, end);
    if (tokens.size() == 0 || tokens[0][0] == '#') return true;
    string cmd(tokens[0]);
    if (cmd == "delay") {
        if (tokens.size() != 2) {
            fprintf(stderr,"Invalid delay\n");
            return false;
        }
        size_t micros = (size_t) strtoull(tokens[1].c_str(),NULL,10);
        usleep(micros);
        return true;
    } else if (cmd == "run") {
        if (tokens.size() != 2) {
            fprintf(stderr,"Invalid run\n");
            return false;
        }
        return script(tokens[1]);
    } else if (cmd == "i2c") {
        string bus(tokens[1]);
        if (bus == "cd") { // i2c cd femb coldata chip page addr data
            uint8_t femb_idx = (uint8_t)strtoull(tokens[2].c_str(),NULL,10);
            uint8_t coldata_idx = (uint8_t)strtoull(tokens[3].c_str(),NULL,10);
            uint8_t chip_addr = (uint8_t)strtoull(tokens[4].c_str(),NULL,16);
            uint8_t reg_page = (uint8_t)strtoull(tokens[5].c_str(),NULL,16);
            uint8_t reg_addr = (uint8_t)strtoull(tokens[6].c_str(),NULL,16);
            uint8_t data = (uint8_t)strtoull(tokens[7].c_str(),NULL,16);
            cdpoke(femb_idx, coldata_idx, chip_addr, reg_page, reg_addr, data);
            return true;
        } else {
            i2c_t *i2c_bus;
            if (bus == "sel") {  // i2c sel chip addr data [...]
                i2c_bus = &this->selected_i2c;
            } else if (bus == "pwr") { // i2c pwr chip addr data [...]
                i2c_bus = &this->power_i2c;
            } else {
                fprintf(stderr,"Invalid i2c bus selection: %s\n", tokens[1].c_str());
                return false;
            }
            uint8_t chip = (uint8_t)strtoull(tokens[2].c_str(),NULL,16);
            uint8_t addr = (uint8_t)strtoull(tokens[3].c_str(),NULL,16);
            if (tokens.size() < 5) {
                fprintf(stderr,"Invalid arguments to i2c\n");
            } else if (tokens.size() > 5) {
                size_t size = tokens.size() - 4;
                uint8_t *buf = new uint8_t[size];
                for (size_t i = 0; i < size; i++) {
                    buf[i] = (uint8_t)strtoull(tokens[4+i].c_str(),NULL,16);
                }
                i2c_block_write(i2c_bus, chip, addr, buf, size);
                delete [] buf;
                return true;
            } else {
                uint8_t data = (uint8_t)strtoull(tokens[4].c_str(),NULL,16);
                i2c_reg_write(i2c_bus, chip, addr, data);
                return true;
            }
        }
    } else if (cmd == "mem") {
        if (tokens.size() == 3) { // mem addr value
            uint32_t addr = strtoull(tokens[1].c_str(),NULL,16);
            uint32_t value = strtoull(tokens[2].c_str(),NULL,16);
            poke(addr, value);
            return true;
        } else if (tokens.size() == 4) { // mem addr value mask
            uint32_t addr = strtoull(tokens[1].c_str(),NULL,16);
            uint32_t value = strtoull(tokens[2].c_str(),NULL,16);
            uint32_t mask = strtoull(tokens[3].c_str(),NULL,16);
            uint32_t prev = peek(addr);
            poke(addr, (prev & (~mask)) | (value & mask));
            return true;
        } else {
            fprintf(stderr,"Invalid arguments to mem\n");
        }
    } else if (cmd == "fast") {
        if (tokens.size() != 2) {
            fprintf(stderr,"Invalid arguments to fast\n");
            return false;
        }
        string fast(tokens[1]);
        if (fast == "reset") {
            FEMB::fast_cmd(FAST_CMD_RESET);
        } else if (cmd == "act") {
            FEMB::fast_cmd(FAST_CMD_ACT);
        } else if (fast == "sync") {
            FEMB::fast_cmd(FAST_CMD_SYNC);
        } else if (fast == "edge") {
            FEMB::fast_cmd(FAST_CMD_EDGE);
        } else if (fast == "idle") {
            FEMB::fast_cmd(FAST_CMD_IDLE);
        } else if (fast == "edge_act") {
            FEMB::fast_cmd(FAST_CMD_EDGE_ACT);
        } else {
            fprintf(stderr,"Unknown fast command: %s\n",fast.c_str());
            return false;
        }
        return true;
    } else {
        fprintf(stderr,"Invalid script command: %s\n", tokens[0].c_str());
    }
    return false;
}

bool WIB::script(string script, bool file) {
    if (file) {
        ifstream fin(script);
        if (!fin.is_open()) {
            fin.clear();
            fin.open("scripts/"+script);
            if (!fin.is_open()) {
                fin.clear();
                fin.open("/etc/wib/"+script);        
                if (!fin.is_open()) {
                    return false;
                }
            }
        }
        string str((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
        fin.close();
        script = str;
    }
    istringstream iss(script);

    for (string line; getline(iss, line); ) {
        printf("%s\n",line.c_str());
        if (!script_cmd(line)) return false;
    }
    return true;
}

void WIB::i2c_select(uint8_t device) {
    uint32_t next = io_reg_read(&this->regs,0x04/4);
    next = (next & 0xFFFFFFF0) | (device | 0xF);
    io_reg_write(&this->regs,0x04/4,next);
}

bool WIB::read_daq_spy(void *buf0, void *buf1) {
    uint32_t next = io_reg_read(&this->regs,0x04/4);
    uint32_t mask = 0;
    if (buf0) mask |= (1 << 0);
    if (buf1) mask |= (1 << 1);
    next |= (mask << 6);
    io_reg_write(&this->regs,0x04/4,next);
    bool success = false;
    for (size_t i = 0; i < 10; i++) { // try for 10 ms (should take max 4)
        usleep(1000);
        if ((io_reg_read(&this->regs,0x80/4) & mask) == mask) {
            success = true;
            break;
        }
    }
    if (buf0) memcpy(buf0,this->daq_spy[0],DAQ_SPY_SIZE);
    if (buf1) memcpy(buf1,this->daq_spy[1],DAQ_SPY_SIZE);
    return success;
}

uint32_t WIB::peek(size_t addr) {
    #ifndef SIMULATION
    size_t page_addr = (addr & ~(sysconf(_SC_PAGESIZE)-1));
    size_t page_offset = addr-page_addr;

    int fd = open("/dev/mem",O_RDWR);
    void *ptr = mmap(NULL,sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(addr & ~(sysconf(_SC_PAGESIZE)-1)));

    return *((uint32_t*)((char*)ptr+page_offset));
    
    munmap(ptr,sysconf(_SC_PAGESIZE));
    close(fd);
    #else
    return 0x0;
    #endif
}

void WIB::poke(size_t addr, uint32_t val) {
    #ifndef SIMULATION
    size_t page_addr = (addr & ~(sysconf(_SC_PAGESIZE)-1));
    size_t page_offset = addr-page_addr;

    int fd = open("/dev/mem",O_RDWR);
    void *ptr = mmap(NULL,sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(addr & ~(sysconf(_SC_PAGESIZE)-1)));

    *((uint32_t*)((char*)ptr+page_offset)) = val;
    
    munmap(ptr,sysconf(_SC_PAGESIZE));
    close(fd);
    #endif
}

uint8_t WIB::cdpeek(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr) {
    return this->femb[femb_idx]->i2c_read(coldata_idx,chip_addr,reg_page,reg_addr);
}

void WIB::cdpoke(uint8_t femb_idx, uint8_t coldata_idx, uint8_t chip_addr, uint8_t reg_page, uint8_t reg_addr, uint8_t data) {
    this->femb[femb_idx]->i2c_write(coldata_idx,chip_addr,reg_page,reg_addr,data);
}

bool WIB::reboot() {
    int ret = system("reboot");
    return WEXITSTATUS(ret) == 0;
}

bool WIB::update(const string &root_archive, const string &boot_archive) {
    ofstream out_boot("/home/root/boot_archive.tar.gz", ofstream::binary);
    out_boot.write(boot_archive.data(),boot_archive.size());
    out_boot.close();
    printf("Expanding boot archive (%0.1f MB)\n",boot_archive.size()/1024.0/1024.0);
    int ret1 = system("wib_update.sh /home/root/boot_archive.tar.gz /boot");
    
    ofstream out_root("/home/root/root_archive.tar.gz", ofstream::binary);
    out_root.write(root_archive.data(),root_archive.size());
    out_root.close();
    printf("Expanding root archive (%0.1f MB)\n",root_archive.size()/1024.0/1024.0);
    int ret2 = system("wib_update.sh /home/root/root_archive.tar.gz /");
    
    return WEXITSTATUS(ret1) == 0 && WEXITSTATUS(ret2) == 0;
}


bool WIB::configure_wib(wib::ConfigureWIB &conf) {
    return false; //FIXME do stuff in this method
}

bool WIB::configure_femb(wib::ConfigureFEMB &conf) {
    uint8_t femb_idx = conf.index();
    bool enabled = conf.enabled();
    return false; //FIXME do stuff in this method
}

bool WIB::read_sensors(wib::Sensors &sensors) {
   
   printf("Activating I2C_SENSOR bus\n");
   i2c_select(I2C_SENSOR);
    
   printf("Enabling voltage sensors\n");
   uint8_t buf[1] = {0x7};
   i2c_write(&this->selected_i2c,0x70,buf,1); //
   // 0x48 LTC2991
   // 0x4C 0x4E  LTC2990
   
   enable_ltc2990(&this->selected_i2c,0x4E);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4E ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&this->selected_i2c,0x4E,i));
   }
   printf("LTC2990 0x4E Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&this->selected_i2c,0x4E,5)+2.5);
   
   enable_ltc2990(&this->selected_i2c,0x4C);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4C ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&this->selected_i2c,0x4C,i));
   }
   printf("LTC2990 0x4C Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&this->selected_i2c,0x4C,5)+2.5);
   
   enable_ltc2991(&this->selected_i2c,0x48);
   for (uint8_t i = 1; i < 8; i++) {
       printf("LTC2991 0x48 ch%i -> %0.2f V\n",i,0.00030518*read_ltc2991_value(&this->selected_i2c,0x48,i));
   }
   printf("LTC2991 0x48 T -> %0.2f C\n",0.0625*read_ltc2991_value(&this->selected_i2c,0x48,8));
   printf("LTC2991 0x48 Vcc -> %0.2f V\n",0.00030518*read_ltc2991_value(&this->selected_i2c,0x48,9)+2.5);
   
   // 0x49 0x4D 0x4A supposedly are AD7414
   printf("AD7414 0x49 temp %i\n", read_ad7414_temp(&this->selected_i2c,0x49));
   printf("AD7414 0x4D temp %i\n", read_ad7414_temp(&this->selected_i2c,0x4D));
   printf("AD7414 0x4A temp %i\n", read_ad7414_temp(&this->selected_i2c,0x4A));
   
   // 0x15 LTC2499
   printf("Reading temperature sensors\n");
   for (uint8_t i = 0; i < 7; i++) {
       printf("LTC2499 ch%i -> %0.14f\n",i,read_ltc2499_temp(&this->selected_i2c,i));
   }
   
   return true;
}

