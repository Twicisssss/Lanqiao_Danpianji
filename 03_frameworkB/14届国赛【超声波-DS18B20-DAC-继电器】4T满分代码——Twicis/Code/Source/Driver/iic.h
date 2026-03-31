#include <STC15F2K60S2.H>
#include <intrins.H>
sbit scl=P2^0;
sbit sda=P2^1;
void da_write(unsigned char dat);
unsigned char ad_read(unsigned char addr);
