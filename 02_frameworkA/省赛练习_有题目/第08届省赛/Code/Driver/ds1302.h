/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <intrins.H>

sbit SCK=P1^7;
sbit SDA=P2^3;
sbit RST=P1^3;


void Set_RTC(unsigned char *ucRtc);
void Read_RTC(unsigned char *ucRtc);







