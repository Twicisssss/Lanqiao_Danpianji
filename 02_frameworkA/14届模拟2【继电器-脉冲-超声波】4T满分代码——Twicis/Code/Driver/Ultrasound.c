#include <Ultrasound.H>
void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
void US_Init()
{
	unsigned char j;
	for(j=0;j<8;j++)
	{
		US_TX=1;
		Delay12us();
		US_TX=0;
		Delay12us();
	}
}
unsigned char US_Distance_Get()
{
	unsigned int time;
	CMOD = 0x00;			//设置定时器模式
	CL = 0x00;				//设置定时初始值
	CH = 0x00;				//设置定时初始值
	EA=0;
	US_Init();
	EA=1;
	CR=1;
	while((US_RX==1)&&(CF==0));
	CR=0;
	if(CF==0)
	{
		time=CH<<8|CL;
		return time*0.017;
	}
	else
	{
		CF=0;
		return 0;
	}
}

