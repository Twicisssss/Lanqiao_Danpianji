#include <STC15F2K60S2.H>
#include <intrins.H>
sbit SCK = P1^7;
sbit SDA = P2^3;
sbit RST = P1^3;
/*时钟设置*/
void RTC_Set(unsigned char *Rtc);
/*时钟读取*/
void RTC_Read(unsigned char *Rtc);
/*日期设置*/
void DATE_Set(unsigned char *Date);
/*日期读取*/
void DATE_Read(unsigned char *Date);
