#include <STC15F2K60S2.H>
#include <intrins.H>
sbit SCK=P1^7;
sbit SDA=P2^3;
sbit RST=P1^3;


void Rtc_Read(unsigned char *ucRtc);
void Rtc_Set(unsigned char *ucRtc);
