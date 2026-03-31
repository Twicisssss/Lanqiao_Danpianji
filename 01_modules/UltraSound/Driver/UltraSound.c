#include <UltraSound.H>

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void US_Emission()
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

unsigned char US_Length_Read(void)
{
	unsigned int time;
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0;				//设置定时初始值
	TH1 = 0;				//设置定时初始值
	US_Emission();
	TR1 = 1;				//定时器1开始计时
	while((US_RX==1)&&(TF1==0));
	TR1=0;
	if(TF1==0)//定时器未溢出
	{
		time=TH1<<8|TL1;
		return (time*0.017);
	}
	else//定时器溢出
	{
		TF1=0;
		return 0;
	}
}


