#include <led.H>
unsigned char led=0x00;
unsigned char led_old=0xff;
void led_disp(unsigned char *pucLed)
{
	unsigned char temp;
	led=0x00;
	led=(pucLed[0]<<0)|(pucLed[1]<<1)|(pucLed[2]<<2)|(pucLed[3]<<3)|
		(pucLed[4]<<4)|(pucLed[5]<<5)|(pucLed[6]<<6)|(pucLed[7]<<7);
	
	if(led!=led_old)
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
		brm&=~(0x10);
	
	if(brm!=brm_old)
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
void motor(bit enable)
{
	unsigned char temp;

	if(enable)
		brm|=0x20;
	else
		brm&=~(0x20);
	
	if(brm!=brm_old)
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
