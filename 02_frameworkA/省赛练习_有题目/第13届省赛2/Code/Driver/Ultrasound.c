#include <Ultrasound.H>
void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
void Ultrasound_Init()
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

//void Timer1_Init(void)		//1毫秒@12.000MHz
unsigned char Ultrasound_Distance_Get()
{
	unsigned int time;
//	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0;				//设置定时初始值
	TH1 = 0;				//设置定时初始值
//	TF1 = 0;				//清除TF1标志
//	TR1 = 1;				//定时器1开始计时
	
	EA=0;
	Ultrasound_Init();
	EA=1;
	TR1 = 1;//开始计时
	while((US_RX==1)&&(TF1==0))//收到超声波RX会置【0】，定时溢出TF1会置【1】。
		;//【没收到返回的波】且【定时器没溢出】就卡死
	TR1 = 0;//开始计时
	if(TF1==0)//若定时没有溢出
	{
		time=(TH1<<8)|TL1;
		return(time*0.017);
	}
	else//若定时已经溢出
	{
		TF1=0;
		return 0;
	}
}

