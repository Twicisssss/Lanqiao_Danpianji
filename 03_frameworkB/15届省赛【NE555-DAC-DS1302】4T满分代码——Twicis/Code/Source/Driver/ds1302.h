#include <STC15F2K60S2.H>
#include <intrins.H>
sbit SCK = P1^7;
sbit SDA = P2^3;
sbit RST = P1^3;
void set_rtc(unsigned char *ucRtc);
void read_rtc(unsigned char *ucRtc);
