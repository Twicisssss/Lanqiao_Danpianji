#include <led.H>
idata unsigned char led=0x00;
idata unsigned char led_old=0xff;
void led_disp(unsigned char *pLed)
{
	unsigned char temp;
	
	led=0x00;
	led=(pLed[0]<<0)|(pLed[1]<<1)|(pLed[2]<<2)|(pLed[3]<<3)|
			(pLed[4]<<4)|(pLed[5]<<5)|(pLed[6]<<6)|(pLed[7]<<7);
	
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
void led_off()
{
	unsigned char temp;
	
	led=0xff;
	
	P0=led;
	
	temp=P2&0x1f;
	temp=temp|0x80;
	P2=temp;
	temp=P2&0x1f;
	P2=temp;		
	
	led_old=0x00;
}

idata unsigned char brm=0x00;
idata unsigned char brm_old=0xff;
void beep(bit enable)
{
	unsigned char temp;
	
	if(enable)
		brm|=0x40;
	else
		brm&=~(0x40);
	
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
//void motor(bit enable)
//{
//	unsigned char temp;
//	
//	if(enable)
//		brm|=0x20;
//	else
//		brm&=~(0x20);
//	
//	if(brm!=brm_old)
//	{
//		P0=brm;
//		
//		temp=P2&0x1f;
//		temp=temp|0xa0;
//		P2=temp;
//		temp=P2&0x1f;
//		P2=temp;		
//		
//		brm_old=brm;
//	}
//}
