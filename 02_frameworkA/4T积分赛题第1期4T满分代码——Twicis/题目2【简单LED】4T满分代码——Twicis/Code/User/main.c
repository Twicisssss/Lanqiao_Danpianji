/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
/*变量声明区域*/
idata unsigned char Led_Pos,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned int Time_1s;
idata bit Time_1s_Flag=0;
idata unsigned int Time_100ms;
idata bit Time_100ms_Flag;
/*Led控制区域*/
void Led_Proc()
{
	Led_Buf[0]=Time_1s_Flag==0?1:0;
	Led_Buf[1]=Time_1s_Flag==1?1:0;
	Led_Buf[7]=Time_100ms_Flag;
}
/*定时器1区域*/
void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA=1;
}
void Timer1_Isr(void) interrupt 3
{
	if(++Led_Pos==8)Led_Pos=0;
	Led_Disp(Led_Pos,Led_Buf[Led_Pos]);
	
	if(++Time_100ms==100)
	{
		Time_100ms=0;
		Time_100ms_Flag^=1;
	}
	if(++Time_1s==1000)
	{
		Time_1s=0;
		Time_1s_Flag^=1;
	}
}
/*主函数区域*/
void main()
{
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Led_Proc();
	}
}