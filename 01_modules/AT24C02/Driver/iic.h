#include <STC15F2K60S2.H>
#include <intrins.H>

sbit scl=P2^0;
sbit sda=P2^1;

/*
static void I2C_Delay(unsigned char n);
void I2CStart(void);
void I2CStop(void);
void I2CSendByte(unsigned char byt);
unsigned char I2CReceiveByte(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(unsigned char ackbit);
*/
void EEPROM_Write(unsigned char* String,unsigned char addr,unsigned char num);
void EEPROM_Read(unsigned char* String,unsigned char addr,unsigned char num);
