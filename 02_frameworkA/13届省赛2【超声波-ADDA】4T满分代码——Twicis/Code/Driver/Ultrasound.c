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
	unsigned char i;
	for(i=0;i<8;i++)
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
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	US_Init();
//	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	while(US_RX==1 && TF0==0);
	TR0 = 0;				//定时器0停止计时
	if(TF0==0)
	{
		time=(TH0<<8)|TL0;
		return (time*0.017);
	}
	else
	{
		TF0 = 0;
		return 0;
	}
}
