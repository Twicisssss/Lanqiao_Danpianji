#include <Beeper.H>
void Beeper(bit enable)
{
	static unsigned char temp=0x00;
	static unsigned char temp_old=0xff;
	
	if(enable)
		temp|=0x40;
	else
		temp&=~(0x40);
	
	if(temp!=temp_old)
	{
		P0=temp;
		P2=P2&0x1f|0xa0;
		P2&=0x1f;
		temp_old=temp;
	}
}