#include <STC15F2K60S2.H>
#include <intrins.H>
sbit scl = P2^0;
sbit sda = P2^1;
unsigned char AD_Read(unsigned char addr);
void DA_Wriet(unsigned char dat);
