#include <STC15F2K60S2.H>
#include <intrins.H>
sbit sda = P2^1;
sbit scl = P2^0;
/*ADC读取*/
unsigned char AD_Read(unsigned char addr);
/*DAC输出*/
void DA_Write(unsigned char dat);
/*EEPROM读取*/
void EEPROM_Read(unsigned char *String,unsigned char addr,unsigned char length);
/*EEPROM写入*/
void EEPROM_Write(unsigned char *String,unsigned char addr,unsigned char length);


