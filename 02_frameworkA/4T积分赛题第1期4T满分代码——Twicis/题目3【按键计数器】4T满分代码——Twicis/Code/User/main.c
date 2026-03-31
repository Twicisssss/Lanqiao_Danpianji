/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <Led.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i;

idata unsigned char Count_Num=0;
idata bit Work_Mode=1;//【0-锁定】【1-解锁】

idata unsigned int Time_2s;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Work_Mode==1)
	{
		if(Key_Down==4)
		{
			if(++Count_Num==101)Count_Num=100;
		}
		else if(Key_Down==5)
		{
			if(--Count_Num==255)Count_Num=0;
		}
		else if(Key_Down==6)
		{
			Count_Num=0;
			Led_Buf[0]=1;
		}
	}
	if(Key_Down==7)
	{
		Work_Mode^=1;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Seg_Buf[5]=Count_Num/100%10;
	Seg_Buf[6]=Count_Num/10%10;
	Seg_Buf[7]=Count_Num/1%10;
	for(i=5;i<7;i++)
	{
		if(Seg_Buf[i]!=0)break;
		Seg_Buf[i]=10;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Led_Buf[7]=!Work_Mode;
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
	if(++Seg_Slow==90)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	
	if(Led_Buf[0]==1)
	{
		if(++Time_2s==2000)
		{
			Time_2s=0;
			Led_Buf[0]=0;
		}
	}
	
}
/*主函数区域*/
void main()
{
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}
