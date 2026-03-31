#include <STC15F2K60S2.H>
#include <intrins.H>
sbit sda = P2^1;
sbit scl = P2^0;
unsigned char AD_Read(unsigned char addr);
void DA_Write(unsigned char dat);

