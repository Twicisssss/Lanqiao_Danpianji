#include <STC15F2K60S2.H>
#include <intrins.H>

sbit SCK=P1^7;
sbit SDA=P2^3;
sbit RST=P1^3;

void Set_Rtc(unsigned char* ucRtc);
void Read_Rtc(unsigned char* ucRtc);

