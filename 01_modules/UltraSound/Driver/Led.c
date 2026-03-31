#include <Led.H>

void Led_Disp(unsigned char addr,enable)
{
	unsigned char temp=0x00;
	unsigned char temp_old=0xff;
	
	if(enable)
		temp|=(0x01<<addr);
	else
		temp&=~(0x01<<addr);
	
	P0=~temp;
	P2=P2&0x1f|0x80;
	P2&=0x1f;
	temp_old=temp;
}
