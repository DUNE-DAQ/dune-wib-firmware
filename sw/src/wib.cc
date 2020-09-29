#include "wib.h"
#include "sensors.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>

using namespace std;

WIB::WIB() {
    io_reg_init(&this->regs,0xA0002000,32);
    io_reg_init(&this->regs,0xA000D000,32);
    i2c_init(&this->i2c,"/dev/i2c-0");
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
    //setup the FEMBs and COLDATA chips
}

uint32_t WIB::peek(size_t addr) {
	size_t page_addr = (addr & ~(sysconf(_SC_PAGESIZE)-1));
	size_t page_offset = addr-page_addr;

    int fd = open("/dev/mem",O_RDWR);
	void *ptr = mmap(NULL,sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(addr & ~(sysconf(_SC_PAGESIZE)-1)));

	return *((uint32_t*)(ptr+page_offset));
}

uint32_t WIB::poke(size_t addr, uint32_t val) {
	size_t page_addr = (addr & ~(sysconf(_SC_PAGESIZE)-1));
	size_t page_offset = addr-page_addr;

    int fd = open("/dev/mem",O_RDWR);
	void *ptr = mmap(NULL,sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(addr & ~(sysconf(_SC_PAGESIZE)-1)));

	*((uint32_t*)(ptr+page_offset)) = val;
	return val;
}

bool WIB::reboot() {
    system("reboot");
    return true;
}

bool WIB::update(const string &root_archive, const string &boot_archive) {
    ofstream out_boot("/home/root/boot_archive.tar.gz", ofstream::binary);
    out_boot.write(boot_archive.data(),boot_archive.size());
    printf("Expanding boot archive (%0.1f MB)\n",boot_archive.size()/1024.0/1024.0);
    system("wib_update.sh /home/root/boot_archive.tar.gz /boot");
    
    ofstream out_root("/home/root/root_archive.tar.gz", ofstream::binary);
    out_root.write(root_archive.data(),root_archive.size());
    printf("Expanding root archive (%0.1f MB)\n",root_archive.size()/1024.0/1024.0);
    system("wib_update.sh /home/root/root_archive.tar.gz /");
    
    //system("reboot");
    return true;
}

bool WIB::read_sensors(wib::Sensors &sensors) {

   
   printf("Activating I2C_SENSOR bus\n");
   io_reg_write(&this->regs,1,0x00000005); //reg1 = 0x5
    
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

