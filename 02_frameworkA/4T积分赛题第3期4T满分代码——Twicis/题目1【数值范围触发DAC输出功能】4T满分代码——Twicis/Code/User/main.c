#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Num=0;

void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Key_Down==4)
	{
		if(++Num==100)Num=99;
	}
	else if(Key_Down==5)
	{
		if(--Num==255)Num=0;
	}
}
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	if(Num!=0 && Num%6==0)
		DA_Write(3*51);
	else
		DA_Write(1*51);
	
	Seg_Buf[6]=Num>=10?Num/10%10:10;
	Seg_Buf[7]=Num/1%10;
	
	
}
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
	if(++Seg_Slow==100)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
}
void main()
{
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
	}
}


