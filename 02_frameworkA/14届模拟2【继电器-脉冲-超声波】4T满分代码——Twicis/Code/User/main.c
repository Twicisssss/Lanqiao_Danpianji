/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <Ultrasound.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i;

idata unsigned char Seg_Show_Mode=0;//【0-频率】【1-距离】
idata unsigned char US_Mode=0;//【0-频率控制】【1-按键控制】
idata bit US_Dis_Once_Flag=0;

idata unsigned char US_Distance;
idata unsigned int Time_F_1s;
idata unsigned int Frequency;

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
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
		break;
		case 5:
			US_Distance=0;
			if(++US_Mode==2)
				US_Mode=0;
		break;
		case 8:
			if(US_Mode==1)
			{
				US_Dis_Once_Flag=1;
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	if(US_Mode==0)
	{
		if(Frequency>1000)
			US_Distance=US_Distance_Get();
		else
			US_Distance=0;
	}
	else
	{
		if(US_Dis_Once_Flag==1)
		{
			US_Distance=US_Distance_Get();
			US_Dis_Once_Flag=0;
		}
	}
	
	Seg_Buf[0]=11;
	Seg_Buf[1]=Seg_Show_Mode+1;
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[5]=0;
			Seg_Buf[2]=Frequency/100000%10;
			Seg_Buf[3]=Frequency/10000%10;
			Seg_Buf[4]=Frequency/1000%10;
			Seg_Buf[5]=Frequency/100%10;
			Seg_Buf[6]=Frequency/10%10;
			Seg_Buf[7]=Frequency/1%10;
			for(i=2;i<7;i++)
			{
				if(Seg_Buf[i]!=0)break;
				Seg_Buf[i]=10;
			}
		break;
		case 1:
			Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=US_Distance/100%10;
			Seg_Point[5]=(US_Distance==0)?0:1;
			Seg_Buf[6]=US_Distance/10%10;
			Seg_Buf[7]=US_Distance/1%10;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	
	Led_Buf[2]=US_Mode==0?1:0;
	
	Relay_Buf=(US_Distance>65);
	Led_Buf[7]=(Relay_Buf==1)?Time_100ms_Flag:0;
}
/*定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
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
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
	
	
//	if(++Time_F_1s==1000)
//	{
//		Time_F_1s=0;
//		Frequency=(TH0<<8)|TL0;
//		TH0=TL0=0;
//	}
	if(++Time_F_1s==100)
	{
		Time_F_1s=0;
		Frequency=((TH0<<8)|TL0);
		Frequency=Frequency*10;
		TH0=TL0=0;
	}
	
	if(Relay_Buf==1)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			Time_100ms_Flag^=1;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
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
