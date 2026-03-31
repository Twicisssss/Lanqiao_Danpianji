#include <STC15F2K60S2.H>
#include <intrins.H>
sbit sda = P2^1;
sbit scl = P2^0;
unsigned char ad_read(unsigned char addr);
void da_write(unsigned char dat);
