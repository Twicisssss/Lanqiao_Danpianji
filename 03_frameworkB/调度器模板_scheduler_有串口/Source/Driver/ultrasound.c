#include <ultrasound.H>
void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 38;
	while (--i);
}
void us_wave_init(void)
{
	unsigned char us;
	EA=0;
	for(us=0;us<8;us++)
	{
		TX=1;
		Delay12us();
		TX=0;
		Delay12us();
	}
	EA=1;
}
unsigned char us_distance_get()
{
	unsigned int time;
	
	CMOD = 0x00;		//设置定时器模式
	CL = CH = 0;		//设置定时初始值
	us_wave_init();
	CR = 1;				//定时器开始计时
	while((RX==1)&&(CF==0))
		;
	CR = 0;
	if(CF==0)
	{
		time=(CH<<8)|CL;
		return (time*0.017);
	}
	else
	{
		CF=0;
		return 0;
	}
}
