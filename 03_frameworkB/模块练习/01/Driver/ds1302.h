#include <STC15F2K60S2.H>
#include <intrins.H>
sbit RST = P1^3;
sbit SCK = P1^7;
sbit SDA = P2^3;
void rtc_set(unsigned char *ucRtc);
void rtc_read(unsigned char *ucRtc);
