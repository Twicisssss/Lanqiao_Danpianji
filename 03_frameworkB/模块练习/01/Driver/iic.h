#include <STC15F2K60S2.H>
#include <intrins.H>
sbit sda = P2^1;
sbit scl = P2^0;
unsigned char ad_read(unsigned char addr);
void da_write(unsigned char dat);
void eeprom_write(unsigned char *string,unsigned char addr,unsigned char length);
void eeprom_read(unsigned char *string,unsigned char addr,unsigned char length);
