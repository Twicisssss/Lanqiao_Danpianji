#include <STC15F2K60S2.H>
#include <intrins.H>

sbit US_TX=P1^0;
sbit US_RX=P1^1;



unsigned char US_Length_Read(void);

