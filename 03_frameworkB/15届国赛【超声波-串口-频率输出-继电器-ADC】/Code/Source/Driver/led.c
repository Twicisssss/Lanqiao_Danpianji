#include <led.H>
idata unsigned char led_now=0x00;
idata unsigned char led_old=0xff;
void led_disp(unsigned char *ucLed)
{
	unsigned char temp;
	led_now=0x00;
	led_now=(ucLed[0]<<0)|(ucLed[1]<<1)|(ucLed[2]<<2)|(ucLed[3]<<3)|
			(ucLed[4]<<4)|(ucLed[5]<<5)|(ucLed[6]<<6)|(ucLed[7]<<7);
	
	if(led_now!=led_old)
	{
		P0=~led_now;
		
		temp=P2&0x1f;
		temp=temp|0x80;
		P2=temp;
		temp=P2&0x1f;
		P2=temp;
		
		led_old=led_now;
	}
}

idata unsigned char relay_now=0x00;
idata unsigned char relay_old=0xff;
void relay(bit enable)
{
	unsigned char temp;
	
	if (enable)
		relay_now |= 0x10;
	else
		relay_now &= ~(0x10);

	if (relay_now != relay_old)
	{
		P0 = relay_now;
		
		temp=P2&0x1f;
		temp=temp|0xa0;
		P2=temp;
		temp=P2&0x1f;
		P2=temp;
	
		relay_old = relay_now;
	}
}
