#include "wib.h"
#include "sensors.h"

#include <cstdio>
#include <cstdlib>
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
    io_reg_init(&this->regs,0xA0020000,32);
    io_reg_init(&this->leds,0xA00C0000,32);
    i2c_init(&this->i2c,(char*)"/dev/i2c-0");
    i2c_init(&this->power_i2c,(char*)"/dev/i2c-2");
    for (int i = 0; i < 4; i++) {
        this->femb[i] = new FEMB(i);
    }
}

WIB::~WIB() {
    io_reg_free(&this->regs);
    i2c_free(&this->i2c);
    for (int i = 0; i < 4; i++) {
        delete this->femb[i];
    }
}

bool WIB::initialize() {
    clock_config();
    femb_power_config();
    femb_power_set(true);
    femb_serial_reset();
    return false;
}

bool WIB::set_ip(string ip) {
    struct ifreq ifr;
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    struct sockaddr_in *addr = (struct sockaddr_in*)&ifr.ifr_addr;
    inet_pton(AF_INET,ip.c_str(),&addr->sin_addr);
    ioctl(fd, SIOCSIFADDR, &ifr);
    close(fd);
    return true;
}

#include "SI5344-RevD-WIB-Registers.h"

bool WIB::clock_config() {
	int bus = 0;
	// i2c addresses for each synth
	int chip[] = {0x6b};
	// register arrays for each synth
	const si5344_revd_register_t* regs[] = {si5344_revd_registers};

	int i, j;
	int dwords = sizeof(si5344_revd_registers)/sizeof(si5344_revd_register_t); // size of register dump
	int page_now = -1, page_old = -2;

	// FIXME need to select proper i2c device
	i2c_select(I2C_SI5344);

	for (j = 0; j < sizeof(chip)/sizeof(int); j++) {
		page_now = -1;
		page_old = -2;
		for (i = 0; i < dwords; i ++) {
			si5344_revd_register_t rd = regs[j][i];

			// detect page switches
			page_old = page_now;
			page_now = (rd.address >> 8) & 0xff;
			if (page_now != page_old)
				i2c_reg_write(&this->i2c,chip[j],1,page_now);

			i2c_reg_write(&this->i2c,chip[j],rd.address & 0xff,rd.value);
			
			if (i == 2) usleep(300000);
		}
		// set page back to 0
		i2c_reg_write(&this->i2c,chip[j],1,0);
	}
	
    return true;
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

	i2c_block_write(&this->i2c,chip,reg,buffer,2);

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
    uint32_t value = io_reg_read(&this->regs,1);
    value |= (1<<7);
    io_reg_write(&this->regs,1,value);
    value &= ~(1<<7);
    io_reg_write(&this->regs,1,value);
    return true;
}

void WIB::i2c_select(uint8_t device) {
    uint32_t next = io_reg_read(&this->regs,1);
    next = (next & 0xFFFFFFF0) | (device | 0xF);
    io_reg_write(&this->regs,1,next);
}

uint32_t WIB::peek(size_t addr) {
    #ifndef SIMULATION
	size_t page_addr = (addr & ~(sysconf(_SC_PAGESIZE)-1));
	size_t page_offset = addr-page_addr;

    int fd = open("/dev/mem",O_RDWR);
	void *ptr = mmap(NULL,sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(addr & ~(sysconf(_SC_PAGESIZE)-1)));

	return *((uint32_t*)((char*)ptr+page_offset));
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
    printf("Expanding boot archive (%0.1f MB)\n",boot_archive.size()/1024.0/1024.0);
    int ret1 = system("wib_update.sh /home/root/boot_archive.tar.gz /boot");
    
    ofstream out_root("/home/root/root_archive.tar.gz", ofstream::binary);
    out_root.write(root_archive.data(),root_archive.size());
    printf("Expanding root archive (%0.1f MB)\n",root_archive.size()/1024.0/1024.0);
    int ret2 = system("wib_update.sh /home/root/root_archive.tar.gz /");
    
    return WEXITSTATUS(ret1) == 0 && WEXITSTATUS(ret2) == 0;
}

bool WIB::read_sensors(wib::Sensors &sensors) {

   
   printf("Activating I2C_SENSOR bus\n");
   i2c_select(I2C_SENSOR);
    
   printf("Enabling voltage sensors\n");
   uint8_t buf[1] = {0x7};
   i2c_write(&i2c,0x70,buf,1); //
   // 0x48 LTC2991
   // 0x4C 0x4E  LTC2990
   
   enable_ltc2990(&i2c,0x4E);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4E ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&i2c,0x4E,i));
   }
   printf("LTC2990 0x4E Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&i2c,0x4E,5)+2.5);
   
   enable_ltc2990(&i2c,0x4C);
   for (uint8_t i = 1; i < 5; i++) {
       printf("LTC2990 0x4C ch%i -> %0.2f V\n",i,0.00030518*read_ltc2990_value(&i2c,0x4C,i));
   }
   printf("LTC2990 0x4C Vcc -> %0.2f V\n",0.00030518*read_ltc2990_value(&i2c,0x4C,5)+2.5);
   
   enable_ltc2991(&i2c,0x48);
   for (uint8_t i = 1; i < 8; i++) {
       printf("LTC2991 0x48 ch%i -> %0.2f V\n",i,0.00030518*read_ltc2991_value(&i2c,0x48,i));
   }
   printf("LTC2991 0x48 T -> %0.2f C\n",0.0625*read_ltc2991_value(&i2c,0x48,8));
   printf("LTC2991 0x48 Vcc -> %0.2f V\n",0.00030518*read_ltc2991_value(&i2c,0x48,9)+2.5);
   
   // 0x49 0x4D 0x4A supposedly are AD7414
   printf("AD7414 0x49 temp %i\n", read_ad7414_temp(&i2c,0x49));
   printf("AD7414 0x4D temp %i\n", read_ad7414_temp(&i2c,0x4D));
   printf("AD7414 0x4A temp %i\n", read_ad7414_temp(&i2c,0x4A));
   
   // 0x15 LTC2499
   printf("Reading temperature sensors\n");
   for (uint8_t i = 0; i < 7; i++) {
       printf("LTC2499 ch%i -> %0.14f\n",i,read_ltc2499_temp(&i2c,i));
   }
   
   return true;
}

