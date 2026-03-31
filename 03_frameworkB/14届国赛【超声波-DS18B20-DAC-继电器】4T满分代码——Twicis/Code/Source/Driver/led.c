#include <led.H>
unsigned char led=0x00;
unsigned char led_old=0xff;
void led_disp(unsigned char *pLedBuf)
{
	unsigned char temp;
	led=0x00;
	led=(pLedBuf[0]<<0)|(pLedBuf[1]<<1)|(pLedBuf[2]<<2)|(pLedBuf[3]<<3)|
			(pLedBuf[4]<<4)|(pLedBuf[5]<<5)|(pLedBuf[6]<<6)|(pLedBuf[7]<<7);
	
	if(led_old!=led)
	{
		P0=~led;
		
		temp=P2&0x1f;
		temp=temp|0x80;
		P2=temp;
		temp=P2&0x1f;
		P2=temp;
		
		led_old=led;
	}
}

unsigned char brm=0x00;
unsigned char brm_old=0xff;
void relay(bit enable)
{
	unsigned char temp;
	if(enable)
		brm|=0x10;
	else
		brm&=~0x10;
	
	if(brm_old!=brm)
	{
		P0=brm;
		
		temp=P2&0x1f;
		temp=temp|0xa0;
		P2=temp;
		temp=P2&0x1f;
		P2=temp;
		
		brm_old=brm;
	}
}
