/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Key.H>
#include <Seg.H>
#include <Led.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned int Time_1000ms=0;
unsigned int Frequency=0;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Seg_Buf[0]=Frequency/10000%10;
	Seg_Buf[1]=Frequency/1000%10;
	Seg_Buf[2]=Frequency/100%10;
	Seg_Buf[3]=Frequency/10%10;
	Seg_Buf[4]=Frequency/1%10;	
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=1;
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
	if(++Key_Slow==20)Key_Slow=0;
	if(++Seg_Slow==200)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	if(++Time_1000ms==1000)
	{
		Time_1000ms=0;
		Frequency=(TH0<<8)|TL0;
		TL0 = 0x00;				//设置定时初始值
		TH0 = 0x00;				//设置定时初始值
	}
	
	
}
/*定时器0区域*/
void Timer0_Init(void)		//0毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
/*全体初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer1_Init();
	Timer0_Init();
}
/*主函数区域*/
void main()
{
	Init_Proc();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}
