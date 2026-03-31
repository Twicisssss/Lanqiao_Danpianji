/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i,j;


idata unsigned char Seg_Show_Mode=0;//【0-信号】【1-温度】
idata bit Change_Mode=0;
idata bit Change_Flag=0;

idata float Temperature;
idata unsigned char Temperature_Int;

idata unsigned int Time_F_1s;
idata unsigned int Frequency;
idata unsigned int Cycle;

idata unsigned int Time_1000ms;

idata unsigned char Time_100ms;
idata bit Time_100ms_Flag;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			Change_Mode=0;
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
		break;
		case 5:
			Change_Mode=1;
			Change_Flag=1;
		break;
		case 9:
			Change_Mode=0;
			Seg_Show_Mode=0;
			Change_Flag=0;
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	Temperature=Temperature_Read();
	Temperature_Int=(unsigned char)Temperature;
	Cycle=1000000/Frequency;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[6]=0;
			Seg_Buf[0]=11;
			if(Change_Mode==0)
			{
				Seg_Buf[1]=Frequency/1000000%10;
				Seg_Buf[2]=Frequency/100000%10;
				Seg_Buf[3]=Frequency/10000%10;
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else
			{
				Seg_Buf[1]=Cycle/1000000%10;
				Seg_Buf[2]=Cycle/100000%10;
				Seg_Buf[3]=Cycle/10000%10;
				Seg_Buf[4]=Cycle/1000%10;
				Seg_Buf[5]=Cycle/100%10;
				Seg_Buf[6]=Cycle/10%10;
				Seg_Buf[7]=Cycle/1%10;
			}
			for(j=1;j<7;j++)
			{
				if(Seg_Buf[j]!=0)break;
				Seg_Buf[j]=10;
			}
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(Change_Mode==0)
			{
				Seg_Buf[5]=Temperature<10?10:(unsigned char)(Temperature)/10%10;
				Seg_Buf[6]=(unsigned char)(Temperature)/1%10;
				Seg_Point[6]=1;
				Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
			}
			else
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=Temperature_Int<10?10:Temperature_Int/10%10;
				Seg_Point[6]=0;
				Seg_Buf[7]=Temperature_Int/1%10;
			}
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Change_Flag;
	Led_Buf[7]=Temperature>30?Time_100ms_Flag:0;
}
/*定时器0区域*/
void Timer0_Init(void)		//1微秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
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

	if(++Time_F_1s==100)
	{
		Time_F_1s=0;
		Frequency=(TH0<<8)|TL0;
		Frequency=Frequency*10;
		TH0=TL0=0;
	}
	
	if(Change_Flag==1)
	{
		if(++Time_1000ms==1000)
		{
			Time_1000ms=0;
			Change_Flag=0;
		}
	}
	
	if(Temperature>30)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			Time_100ms_Flag^=1;
		}
	}
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
	
}
/*初始化区域*/
void Init_Proc()
{
	while(Temperature_Read()==85);
	Sys_Init();
	Timer0_Init();
	Timer1_Init();
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
