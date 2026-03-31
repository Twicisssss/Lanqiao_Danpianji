#include <STC15F2K60S2.H>
#include <intrins.H>
sbit sda = P2^1;
sbit scl = P2^0;

void EEPROM_Write(unsigned char *String,unsigned char addr,unsigned char length);
void EEPROM_Read(unsigned char *String,unsigned char addr,unsigned char length);

