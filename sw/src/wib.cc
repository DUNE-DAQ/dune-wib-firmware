#include "wib.h"
#include "unpack.h"
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

#ifdef SIMULATION
#include <cmath>
#endif

using namespace std;

WIB::WIB() {
    io_reg_init(&this->regs,CTRL_REGS,0x10000/4);
    i2c_init(&this->selected_i2c,(char*)"/dev/i2c-0");
    i2c_init(&this->femb_pwr_i2c,(char*)"/dev/i2c-1"); //FIXME these devices appear to be on /dev/i2c-2 ...
    i2c_init(&this->femb_en_i2c,(char*)"/dev/i2c-2");
    #ifdef SIMULATION
    this->daq_spy_fd = -1;
    this->daq_spy[0] = new char[DAQ_SPY_SIZE];
    this->daq_spy[1] = new char[DAQ_SPY_SIZE];
    fake_data((frame14*)this->daq_spy[0],DAQ_SPY_SIZE/sizeof(frame14));
    fake_data((frame14*)this->daq_spy[1],DAQ_SPY_SIZE/sizeof(frame14));
    #else
    this->daq_spy_fd = open("/dev/mem",O_RDWR);
    this->daq_spy[0] = mmap(NULL,DAQ_SPY_SIZE,PROT_READ,MAP_SHARED,this->daq_spy_fd,DAQ_SPY_0);
    this->daq_spy[1] = mmap(NULL,DAQ_SPY_SIZE,PROT_READ,MAP_SHARED,this->daq_spy_fd,DAQ_SPY_1);
    #endif
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
}

bool WIB::initialize() {
    bool success = true;
    int ret;
    ret = system("ip link set eth0 down");
    if (WEXITSTATUS(ret) != 0) {
        glog.log("failed to bring down eth0\n");
        success = false;
    }
    string eth0_mac("ip link set dev eth0 address "+crate_mac());
    ret = system(eth0_mac.c_str());
    if (WEXITSTATUS(ret) != 0) {
        glog.log("failed to set eth0 mac\n");
        success = false;
    }
    ret = system("ip link set eth0 up");
    if (WEXITSTATUS(ret) != 0) {
        glog.log("failed to bring up eth0\n");
        success = false;
    }
    string eth0_conf("ip addr add "+crate_ip()+" dev eth0");
    ret = system(eth0_conf.c_str());
    if (WEXITSTATUS(ret) != 0) {
        glog.log("failed to assign IP to eth0\n");
        success = false;
    }
    string gw_ip = gateway_ip();
    if (gw_ip.length() > 0) {
        string route_conf("route add default gw "+gw_ip+" eth0");
        ret = system(route_conf.c_str());
        if (WEXITSTATUS(ret) != 0) {
            glog.log("failed to assign default route\n");
            success = false;
        }
    }
    
    return success;
}

bool WIB::reset_timing_endpoint() {
    bool success = true;
    if (!pll_initialized) {
        glog.log("Configuring PLL\n");
        success &= script("conf_pll_timing");
        if (success) {
            pll_initialized = true;
        } else {
            glog.log("Failed to configure PLL\n");
            return false;
        }
    } else {
        glog.log("PLL already configured\n");
    }
    if (backplane_slot_num() == 0xF) {
        glog.log("Slot number is 0xF; assuming there is no backplane.\n");
        glog.log("Using timing signal from SFP");
        io_reg_write(&this->regs,REG_FW_CTRL,(1<<5),(1<<5));
    } else {
        glog.log("Using timing signal from backplane");
        io_reg_write(&this->regs,REG_FW_CTRL,(0<<5),(1<<5));
    }
    glog.log("Resetting timing endpoint\n");
    success &= script("pll_sticky_clear");
    uint32_t value = timing_addr(); //low 8 bits are addr 
    io_reg_write(&this->regs,REG_TIMING,(1<<28)|value); // bit 28 is reset bit
    usleep(2000000);
    io_reg_write(&this->regs,REG_TIMING,value); 
    return success;
}

bool WIB::is_endpoint_locked() {
    //read firmware timing endpoint status
    uint32_t ept_status = io_reg_read(&this->regs, REG_ENDPOINT_STATUS);
    return (ept_status & 0x10F) == 0x108; // ts_ready && ept_locked
}

void WIB::felix_tx_reset() {
    glog.log("Resetting FELIX transmitter\n");
    //assert and release RESET bits for both links
    //FIXME need firmware documentation to better understand this
    io_reg_write(&this->regs,REG_FELIX_CTRL,0x00000311);
    io_reg_write(&this->regs,REG_FELIX_CTRL,0x00000300);
    felix_initialized = true;
}

// read a single field from a file, stripping left and right whitespace
string read_and_strip(ifstream &fin) {
    string param((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    param.erase(param.find_last_not_of(" \t\n\r") + 1);
    param.erase(0,param.find_first_not_of(" \t\n\r"));
    return param;
}

string WIB::crate_mac() {
    ifstream fin("/etc/wib/mac");
    if (fin.is_open()) {
        const string &ip = read_and_strip(fin);
        glog.log("Using MAC %s from /etc/wib/mac\n",ip.c_str());
        return ip;
    }
    glog.log("Using default IP: 00:0a:35:00:22:01\n");
    glog.log("Create /etc/wib/mac containing desired MAC to override\n");
    return "00:0a:35:00:22:01"; 
}

string WIB::crate_ip() {
    ifstream fin("/etc/wib/ip");
    if (fin.is_open()) {
        const string &ip = read_and_strip(fin);
        glog.log("Using IP %s from /etc/wib/ip\n",ip.c_str());
        return ip;
    }
    glog.log("Using default IP: 192.168.121.1/24\n");
    glog.log("Create /etc/wib/ip containing desired IP to override\n");
    return "192.168.121.1/24"; 
}

string WIB::gateway_ip() {
    ifstream fin("/etc/wib/gateway");
    if (fin.is_open()) {
        const string &ip = read_and_strip(fin);
        glog.log("Using gateway %s from /etc/wib/gateway\n",ip.c_str());
        return ip;
    }
    glog.log("Using no default gateway\n"); //iceberg01
    glog.log("Create /etc/wib/gateway containing desired gateway to override\n");
    return "";
}

uint8_t WIB::timing_addr() {
    //FIXME 8 bits is _not enough bits_ for unique WIB addresses in DUNE
    //but this is the address size for the timing endpoint code in firmware
    return ((backplane_crate_num() << 3) | (backplane_slot_num() & 0x7)) & 0xFF;
}

uint8_t WIB::backplane_crate_num() {
    uint32_t addr_reg = io_reg_read(&this->regs,REG_BACKPLANE_ADDR);
    return (uint8_t)((addr_reg>>4) & 0xF); //FIXME not enough bits
}

uint8_t WIB::backplane_slot_num() {
    uint32_t addr_reg = io_reg_read(&this->regs,REG_BACKPLANE_ADDR);
    return (uint8_t)(addr_reg & 0xF);
}

void WIB::set_fake_time(uint64_t time) {
    io_reg_write(&this->regs,REG_FAKE_TIME_L,(uint32_t)(time&0xFFFFFFFF)); //set 4 low bytes
    io_reg_write(&this->regs,REG_FAKE_TIME_H,(uint32_t)((time>>32)&0xFFFFFFFF)); //set 4 high bytes
    io_reg_write(&this->regs,REG_FAKE_TIME_CTRL,0); //disable FTS
}

void WIB::start_fake_time() {
    io_reg_write(&this->regs,REG_FAKE_TIME_CTRL,2); //enable FTS
}

bool WIB::femb_power_reg_ctrl(uint8_t femb_id, uint8_t regulator_id, double voltage) {
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
    glog.log("Configuring front end power\n");
    glog.log("\tdc2dc_o1 %0.2f\n",dc2dc_o1);
    glog.log("\tdc2dc_o2 %0.2f\n",dc2dc_o2);
    glog.log("\tdc2dc_o3 %0.2f\n",dc2dc_o3);
    glog.log("\tdc2dc_o4 %0.2f\n",dc2dc_o4);
    glog.log("\tDO_A0 %0.2f\n",ldo_a0);
    glog.log("\tDO_A1 %0.2f\n",ldo_a1);

    // use the stored voltage config
    for (int i = 0; i <= 3; i++) {
        femb_power_reg_ctrl(i, 0, dc2dc_o1); //dc2dc_O1
        femb_power_reg_ctrl(i, 1, dc2dc_o2); //dc2dc_O2
        femb_power_reg_ctrl(i, 2, dc2dc_o3); //dc2dc_O2
        femb_power_reg_ctrl(i, 3, dc2dc_o4); //dc2dc_O2
        femb_power_reg_ctrl(i, 4, ldo_a0); //ldo_A0
        femb_power_reg_ctrl(i, 5, ldo_a1); //ldo_A1
    }
    
    // configure all pins as outputs for regulator enablers
//    i2c_reg_write(&this->femb_en_i2c, 0x23, 0xC, 0);
//    i2c_reg_write(&this->femb_en_i2c, 0x23, 0xD, 0);
 //   i2c_reg_write(&this->femb_en_i2c, 0x23, 0xE, 0);
//i2c_reg_write(&this->femb_en_i2c, 0x22, 0xC, 0);
    i2c_reg_write(&this->femb_en_i2c, 0x22, 0xD, 0);
    i2c_reg_write(&this->femb_en_i2c, 0x22, 0xE, 0);
   
    i2c_reg_write(&this->femb_en_i2c, 0x23, 0x0C, 0x08);
    i2c_reg_write(&this->femb_en_i2c, 0x23, 0x0D, 0x08);
    i2c_reg_write(&this->femb_en_i2c, 0x23, 0x0E, 0x08);
    i2c_reg_write(&this->femb_en_i2c, 0x22, 0x0C, 0x08);

    return true;
}

bool WIB::configure_power(double dc2dc_o1, double dc2dc_o2, double dc2dc_o3, double dc2dc_o4, double ldo_a0, double ldo_a1) {
    glog.log("Storing new power settings\n");
    this->dc2dc_o1 = dc2dc_o1;
    this->dc2dc_o2 = dc2dc_o2;
    this->dc2dc_o3 = dc2dc_o3;
    this->dc2dc_o4 = dc2dc_o4;
    this->ldo_a0 = ldo_a0;
    this->ldo_a1 = ldo_a1;
    glog.log("Resetting frontend\n");
    return reset_frontend();
}

bool WIB::femb_power_en_ctrl(int femb_idx, uint8_t port_en) {
    //PWR_BIAS_EN enabled if any FEMB regulator is ON
    if (port_en != 0x0 || frontend_power[0] || frontend_power[1] || frontend_power[2] || frontend_power[3]) {
        if (i2c_reg_read(&this->femb_en_i2c, 0x22, 0x5) != 0x1) {
            i2c_reg_write(&this->femb_en_i2c, 0x22, 0x5, 0x1);
            usleep(100000);
        }
    } else {
        if (i2c_reg_read(&this->femb_en_i2c, 0x22, 0x5) != 0x0) {
            i2c_reg_write(&this->femb_en_i2c, 0x22, 0x5, 0x0);
            usleep(100000);
        }
    }
    uint8_t i2c_addr;
    uint8_t i2c_reg;
    switch (femb_idx) {
        case 0:
            i2c_addr = 0x23;
            i2c_reg = 0x4;
            break;
        case 1:
            i2c_addr = 0x23;
            i2c_reg = 0x5;
            break;
        case 2:
            i2c_addr = 0x23;
            i2c_reg = 0x6;
            break;
        case 3:
            i2c_addr = 0x22;
            i2c_reg = 0x4;
            break;
        default:
            return false;
    }
    i2c_reg_write(&this->femb_en_i2c, i2c_addr, i2c_reg, port_en);
    usleep(100000);
    frontend_power[femb_idx] = port_en != 0; // FEMB is "ON" if any regulators are ON
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
            glog.log("Invalid delay\n");
            return false;
        }
        size_t micros = (size_t) strtoull(tokens[1].c_str(),NULL,10);
        usleep(micros);
        return true;
    } else if (cmd == "run") {
        if (tokens.size() != 2) {
            glog.log("Invalid run\n");
            return false;
        }
        return script(tokens[1]);
    } else if (cmd == "i2c") {
        string bus(tokens[1]);
        i2c_t *i2c_bus;
        if (bus == "sel") {  // i2c sel chip addr data [...]
            i2c_bus = &this->selected_i2c;
        } else if (bus == "pwr") { // i2c pwr chip addr data [...]
            i2c_bus = &this->femb_en_i2c;
        } else {
            glog.log("Invalid i2c bus selection: %s\n", tokens[1].c_str());
            return false;
        }
        uint8_t chip = (uint8_t)strtoull(tokens[2].c_str(),NULL,16);
        uint8_t addr = (uint8_t)strtoull(tokens[3].c_str(),NULL,16);
        if (tokens.size() < 5) {
            glog.log("Invalid arguments to i2c\n");
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
            glog.log("Invalid arguments to mem\n");
        }
    } else {
        return script_extra(tokens);
    }
    return false;
}

bool WIB::script_extra(const vector<string> &tokens) {
    glog.log("Invalid script command: %s\n", tokens[0].c_str());
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
                } else {
                    glog.log("Found /etc/wib/%s on WIB\n",script.c_str());
                }
            } else {
                glog.log("Found scripts/%s on WIB\n",script.c_str());
            }
        } else {
            glog.log("Found full or relative path %s on WIB\n",script.c_str());
        }
        glog.log("Running script: %s\n",script.c_str());
        string str((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
        fin.close();
        script = str;
    } else {
        glog.log("Running remote/generated script\n");
    }
    istringstream iss(script);

    for (string line; getline(iss, line); ) {
        //glog.log("%s\n",line.c_str());
        if (!script_cmd(line)) return false;
    }
    return true;
}

void WIB::i2c_select(uint8_t device) {
    uint32_t next = io_reg_read(&this->regs,REG_FW_CTRL);
    next = (next & 0xFFFFFFF0) | (device & 0xF);
    io_reg_write(&this->regs,REG_FW_CTRL,next);
}

void reorder_frames(const frame14_bitfield_v3 *unordered, const size_t nframes, frame14_bitfield_v3 *ordered) {
    uint64_t min_timestamp = unordered[0].timestamp;
    size_t min_index = 0;
    for (size_t i = 0; i < nframes; i++) {
        if (unordered[i].timestamp < min_timestamp) {
	    glog.log("min timestamp is %llx and this timestamp is %llx for frame %llu\n", min_timestamp, unordered[i].timestamp, i);
            min_timestamp = unordered[i].timestamp;
            min_index = i;
        }
    }
    //force no reorder
    //min_index = 0;
    if (min_index != 0) {
	//glog.log("Had to reorder\n");
        size_t start = nframes-min_index;
        size_t rest = min_index;
        memcpy(ordered,unordered+min_index,start*sizeof(frame14_bitfield_v3));
        memcpy(ordered+start,unordered,rest*sizeof(frame14_bitfield_v3));
    } else {
	glog.log("Did not have to reorder\n");
        memcpy(ordered,unordered,nframes*sizeof(frame14_bitfield_v3));
    }
}

size_t extract_frames(const uint32_t *buf, const size_t words, uint32_t *extracted) {
    constexpr uint32_t SOF = 0x3C;
    constexpr uint32_t IDLE = 0xBC;
    constexpr uint32_t PACKET_LENGTH = 119;
    size_t nframes = 0;
    for (size_t i = 0; i < words; ) {
	 if (i > 261200){
             //glog.log("buf %zx is %lx\n",i,buf[i]);
	 }
         if (buf[i] == SOF) { // start of frame
	     if (i > 261200){
                 //glog.log("start of frame %llu %llu\n",nframes,i);
	     }
            size_t j;
            for (j = i+1; j < i+PACKET_LENGTH; j++) {
		if (i > 261200){
		   // glog.log("buf %zx is %lx\n",j,buf[j]);
		}
                if (buf[j%words] == SOF || buf[j%words] == IDLE) {
                    glog.log("bad frame word\n");
		    i = j; // move i to j (bad frame word)
                    break;
                }
            }
            if (i == j) {
		   glog.log("Wasn't a ful frame\n");
		    continue; // wasn't a full frame (idle or sof found before end)
	    }
            if (buf[(i+PACKET_LENGTH+1)%words] != IDLE) {
		glog.log("118 is %lx, 119 is %lx, 120 is %lx\n", buf[(i+PACKET_LENGTH-1)%words], buf[(i+PACKET_LENGTH)%words], buf[(i+PACKET_LENGTH+1)%words]);
                glog.log("Error in the %lx th(hex) word, not a full frame\n",nframes);
		i++;
                continue; // wasn't a full frame (missing trailing idle)
            }
            // Frame is valid
            if (i+PACKET_LENGTH+1 > words) { //wraps around
		//glog.log("frame wrapped around\n");
                size_t start = words-i;
                size_t rest = (PACKET_LENGTH+1)-start;
                memcpy(extracted+nframes*(PACKET_LENGTH+1),buf+i+1,4*start); //copy start from buf end
                memcpy(extracted+nframes*(PACKET_LENGTH+1)+start,buf,4*rest); //copy rest from buf start
            } else { //one segment 
		//glog.log("one segment");
                memcpy(extracted+nframes*(PACKET_LENGTH+1),buf+i+1,4*(PACKET_LENGTH+1));
            }
            nframes++;
	//glog.log("move to next word\n");
            i += (PACKET_LENGTH+1); // move i to next word
        } else {
            //glog.log("skipped %llu %08x\n",i,buf[i]);
            i++;
        }
    }
    return nframes;
}

bool WIB::read_daq_spy(void *buf0, int *nframes0, void *buf1, int *nframes1, uint8_t trig_code, uint32_t spy_rec_time, uint32_t timeout_ms) {
    uint32_t prev = io_reg_read(&this->regs,REG_FW_CTRL);
    uint32_t mask = 0;
    if (buf0) mask |= (1 << 0);
    if (buf1) mask |= (1 << 1);
    //acquisition start are bits 6 and 7 (one per buffer)
    prev &= (~(mask << 6));
    uint32_t next = prev | (mask << 6);
    glog.log("Starting acquisition...\n");
    io_reg_write(&this->regs,REG_TIMING_CMD_1,trig_code<<16,0xFF0000);
    io_reg_write(&this->regs,REG_DAQ_SPY_REC,spy_rec_time,0x3FFFF);
    io_reg_write(&this->regs,REG_FW_CTRL,next);
    io_reg_write(&this->regs,REG_FW_CTRL,prev);
    bool success = false;
    size_t offset;
    if (trig_code == 0) { // no-trigger software kludge
        // wait 10ms then re-assert reset to freeze buffer
        usleep(10000); 
        io_reg_write(&this->regs,REG_FW_CTRL,next);
        glog.log("Performed asynchronous acquisition\n");
        success = true;
    } else {
        uint32_t last_read;
        int ms;
        for (ms = 0; ms < timeout_ms; ms++) {
            usleep(1000);
            if (((last_read = io_reg_read(&this->regs,REG_DAQ_SPY_STATUS)) & mask) == mask) {
                success = true;
                break;
            }
        }
        if (!success) {
            glog.log("Timed out waiting for buffers to fill: %0X\n",last_read);
        } else {
            glog.log("Acquisition took < %i ms\n",ms);
        }
    }
    char *buf = new char[DAQ_SPY_SIZE];
    char *tmp = new char[DAQ_SPY_SIZE];
    if (buf0) {
        glog.log("Copying spy buffer 0\n");
        memcpy(buf,this->daq_spy[0],DAQ_SPY_SIZE);
        size_t nframes = extract_frames((uint32_t*)buf,(DAQ_SPY_SIZE/4)-200,(uint32_t*)tmp);
        if (nframes0) *nframes0 = nframes;
        glog.log("Found %llu frames in buffer 0\n",nframes);
        reorder_frames((frame14_bitfield_v3*)tmp,nframes,(frame14_bitfield_v3*)buf0);
	//memcpy(tmp,buf0,nframes*sizeof(frame14_bitfield_v1));
        success &= nframes > 0;
    }
    if (buf1) {
        glog.log("Copying spy buffer 1\n");
        memcpy(buf,this->daq_spy[1],DAQ_SPY_SIZE);
        size_t nframes = extract_frames((uint32_t*)buf,(DAQ_SPY_SIZE/4)-200,(uint32_t*)tmp);
        if (nframes1) *nframes1 = nframes;
        glog.log("Found %llu frames in buffer 1\n",nframes);
        reorder_frames((frame14_bitfield_v3*)tmp,nframes,(frame14_bitfield_v3*)buf1);
        success &= nframes > 0;
    }
    delete [] tmp;
    delete [] buf;
    #ifdef SIMULATION
    //generate more "random" data for simulation
    glog.log("Generating random sin/cos data for next acquisiton\n");
    fake_data((frame14*)this->daq_spy[0],DAQ_SPY_SIZE/sizeof(frame14));
    fake_data((frame14*)this->daq_spy[1],DAQ_SPY_SIZE/sizeof(frame14));
    #endif
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

bool WIB::reboot() {
    int ret = system("reboot");
    return WEXITSTATUS(ret) == 0;
}

bool WIB::update(const string &root_archive, const string &boot_archive) {
    ofstream out_boot("/home/root/boot_archive.tar.gz", ofstream::binary);
    out_boot.write(boot_archive.data(),boot_archive.size());
    out_boot.close();
    glog.log("Expanding boot archive (%0.1f MB)\n",boot_archive.size()/1024.0/1024.0);
    int ret1 = system("wib_update.sh /home/root/boot_archive.tar.gz /boot");
    
    ofstream out_root("/home/root/root_archive.tar.gz", ofstream::binary);
    out_root.write(root_archive.data(),root_archive.size());
    out_root.close();
    glog.log("Expanding root archive (%0.1f MB)\n",root_archive.size()/1024.0/1024.0);
    int ret2 = system("wib_update.sh /home/root/root_archive.tar.gz /");
    
    return WEXITSTATUS(ret1) == 0 && WEXITSTATUS(ret2) == 0;
}

bool WIB::read_sensors(wib::GetSensors::Sensors &sensors) {
   
    glog.log("Activating I2C_SENSOR bus\n");
    i2c_select(I2C_SENSOR);

    glog.log("Enabling voltage sensors\n");
    uint8_t buf[1] = {0x7};
    i2c_write(&this->selected_i2c,0x70,buf,1); // enable i2c repeater
    
    // 5V (before)
    // 5V
    // VCCPSPLL_Z_1P2V
    // PS_DDR4_VTT
    enable_ltc2990(&this->selected_i2c,0x4E);
    sensors.clear_ltc2990_4e_voltages();
    for (uint8_t i = 1; i <= 4; i++) {
        double v = 0.00030518*read_ltc2990_value(&this->selected_i2c,0x4E,i);
        glog.log("LTC2990 0x4E ch%i -> %0.2f V\n",i,v);
        sensors.add_ltc2990_4e_voltages(v);
    }
    glog.log("LTC2990 0x4E Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&this->selected_i2c,0x4E,6)+2.5);

    // 1.2 V (before)
    // 1.2 V
    // 3.3 V (before)
    // 3.3 V
    enable_ltc2990(&this->selected_i2c,0x4C);
    sensors.clear_ltc2990_4c_voltages();
    for (uint8_t i = 1; i <= 4; i++) {
        double v = 0.00030518*read_ltc2990_value(&this->selected_i2c,0x4C,i);
        glog.log("LTC2990 0x4C ch%i -> %0.2f V\n",i,v);
        sensors.add_ltc2990_4c_voltages(v);
    }
    glog.log("LTC2990 0x4C Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&this->selected_i2c,0x4C,6)+2.5);

    // In pairs (before,after)
    // 0.85 V
    // 0.9 V
    // 2.5 V
    // 1.8 V
    enable_ltc2991(&this->selected_i2c,0x48);
    sensors.clear_ltc2991_48_voltages();
    for (uint8_t i = 1; i <= 8; i++) {
        double v = 0.00030518*read_ltc2991_value(&this->selected_i2c,0x48,i);
        glog.log("LTC2991 0x48 ch%i -> %0.2f V\n",i,v);
        sensors.add_ltc2991_48_voltages(v);
    }
    glog.log("LTC2991 0x48 Vcc -> %0.2f V\n",0.00030518*read_ltc2991_value(&this->selected_i2c,0x48,10)+2.5);

    // 0x49 0x4D 0x4A are AD7414 temperature sensors
    double t;
    t = read_ad7414_temp(&this->selected_i2c,0x49);
    glog.log("AD7414 0x49 temp %0.1f\n", t);
    sensors.set_ad7414_49_temp(t);
    t = read_ad7414_temp(&this->selected_i2c,0x4D);
    glog.log("AD7414 0x4D temp %0.1f\n", t);
    sensors.set_ad7414_4d_temp(t);
    t = read_ad7414_temp(&this->selected_i2c,0x4A);
    glog.log("AD7414 0x4A temp %0.1f\n", t);
    sensors.set_ad7414_4a_temp(t);

    // 0x15 LTC2499 temperature sensor inputs from LTM4644 for FEMB 0 - 3 and WIB 1 - 3
    start_ltc2499_temp(&this->selected_i2c,0);
    sensors.clear_ltc2499_15_temps();
    for (uint8_t i = 0; i < 7; i++) {
        usleep(175000);
        t = read_ltc2499_temp(&this->selected_i2c,i+1);
        glog.log("LTC2499 ch%i -> %0.14f\n",i,t);
        sensors.add_ltc2499_15_temps(t);
    }

    // FIXME 0x46 an INA226 for DDR current
    
    //FEMB power monitoring
    //FIXME docs suggest these should be on the selected_i2c bus set to 3, but they aren't
    i2c_t *femb_power_mon_i2c = &this->femb_en_i2c; 
    
    uint8_t femb_dc2dc_current_addr[4] = {0x48,0x49,0x4a,0x4b};  //DC2DC 0-3 in pairs for FEMBs 0-3
    uint8_t femb_ldo_current_addr[2] = {0x4c,0x4d}; //LDO femb 0-3 in pairs for LDO 0-1
    uint8_t femb_bias_current_addr[1] = {0x4e}; //BIAS femb 0-3 in pairs 
    
    sensors.clear_femb0_dc2dc_ltc2991_voltages();
    sensors.clear_femb1_dc2dc_ltc2991_voltages();
    sensors.clear_femb2_dc2dc_ltc2991_voltages();
    sensors.clear_femb3_dc2dc_ltc2991_voltages();
    sensors.clear_femb_ldo_a0_ltc2991_voltages();
    sensors.clear_femb_ldo_a0_ltc2991_voltages();
    sensors.clear_femb_bias_ltc2991_voltages();
    
    for (uint8_t i = 0; ; i++) {
        uint8_t addr;
        if (i < 4) {
            glog.log("Reading FEMB%i DC2DC current sensor\n",i);
            addr = femb_dc2dc_current_addr[i];
        } else if (i < 6) {
            glog.log("Reading FEMB LDO %i current\n",i-4);
            addr = femb_ldo_current_addr[i-4];
        } else if (i < 7) {
            glog.log("Reading FEMB bias current\n");
            addr = femb_bias_current_addr[i-6];
        } else {
            break;
        }
        enable_ltc2991(femb_power_mon_i2c,addr);
        for (uint8_t j = 1; j <= 8; j++) {
            double v = 0.00030518*read_ltc2991_value(femb_power_mon_i2c,addr,j);
            glog.log("LTC2991 0x%X ch%i -> %0.2f V\n",addr,j,v);
            switch (i) {
                case 0: sensors.add_femb0_dc2dc_ltc2991_voltages(v); break;
                case 1: sensors.add_femb1_dc2dc_ltc2991_voltages(v); break;
                case 2: sensors.add_femb2_dc2dc_ltc2991_voltages(v); break;
                case 3: sensors.add_femb3_dc2dc_ltc2991_voltages(v); break;
                case 4: sensors.add_femb_ldo_a0_ltc2991_voltages(v); break;
                case 5: sensors.add_femb_ldo_a1_ltc2991_voltages(v); break;
                case 6: sensors.add_femb_bias_ltc2991_voltages(v); break;
            }   
        }
        glog.log("LTC2991 0x%X Vcc -> %0.2f V\n",addr,0.00030518*read_ltc2991_value(femb_power_mon_i2c,addr,10)+2.5);
    }

    return true;
}

bool WIB::read_timing_status(wib::GetTimingStatus::TimingStatus &status) {

    constexpr uint8_t pll_i2c_adr = 0x6b; //address of SI5344 chip

    constexpr uint8_t los_reg = 0x0d; // bit 3:0 los for the four inputs, 7:4 OOF for the inputs
    constexpr uint8_t los_flg_reg = 0x12; // sticky version of 0x0d
    constexpr uint8_t lol_reg = 0x0e; // bit 1: lol, bit 5: HOLD (holdover/free run mode)
    constexpr uint8_t lol_flg_reg = 0x13; // sticky version of 0x0e
    
    //read i2c status registers
    i2c_select(I2C_SI5344);
    uint8_t los_val = i2c_reg_read(&this->selected_i2c, pll_i2c_adr, los_reg);
    uint8_t los_flg_val = i2c_reg_read(&this->selected_i2c, pll_i2c_adr, los_flg_reg);
    uint8_t lol_val = i2c_reg_read(&this->selected_i2c, pll_i2c_adr, lol_reg);
    uint8_t lol_flg_val = i2c_reg_read(&this->selected_i2c, pll_i2c_adr, lol_flg_reg);
    
    //read firmware timing endpoint status
    uint32_t ept_status = io_reg_read(&this->regs, REG_ENDPOINT_STATUS);
    
    status.set_los_val(los_val);
    status.set_los_flg_val(los_flg_val);
    status.set_lol_val(lol_val);
    status.set_lol_flg_val(lol_flg_val);
    status.set_ept_status(ept_status);
    
    return true;
}

uint32_t WIB::read_fw_timestamp() {
    return io_reg_read(&this->regs,REG_FW_TIMESTAMP);
}

bool WIB::calibrate() {
    glog.log("Calibrate not implemented\n");
    return false;
}
