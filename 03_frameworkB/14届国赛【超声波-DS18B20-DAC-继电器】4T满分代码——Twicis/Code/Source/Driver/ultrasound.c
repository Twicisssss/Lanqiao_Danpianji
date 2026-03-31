#include <ultrasound.H>

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 38;
	while (--i);
}
void us_init(void)
{
	unsigned char k;
	for(k=0;k<8;k++)
	{
		TX=1;
		Delay12us();
		TX=0;
		Delay12us();
	}
}

unsigned char us_distance_get(void)
{
	unsigned int time;
	CMOD = CL = CH = 0x00;//设置定时初始值
	
	EA=0;
	us_init();
	EA=1;
	
	CR = 1;				//定时器开始计时
	while((CF==0)&&(RX==1))
		;
	CR = 0;				//定时器开始计时
	
	if(CF==0)
	{
		time=(CH<<8)|CL;
		return (time*0.017)+3;
	}
	else
	{
		CF = 0;				//清除CF标志
		return 0;
	}
}

