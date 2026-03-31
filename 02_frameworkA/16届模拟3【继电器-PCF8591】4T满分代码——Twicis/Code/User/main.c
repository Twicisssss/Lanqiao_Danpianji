/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

idata unsigned char Seg_Show_Mode=0;//【0-湿度】【1-参数】【2-时间】

idata float ADC_Rb2_Voltage;
idata unsigned char Humidity;
idata unsigned char Humidity_Parameter=50;

idata unsigned char Interval_Time=3;
idata unsigned int Time_Count;

idata bit Relay_Keep_Flag=0;
idata bit Relay_ON_OFF_Mode=0;//【0-停止】【1-启动】
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
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5:
			Relay_ON_OFF_Mode^=1;
		break;
		case 8:
			if(Seg_Show_Mode==1)
			{
				Humidity_Parameter-=5;
				if(Humidity_Parameter==25)Humidity_Parameter=90;
			}
			else if(Seg_Show_Mode==2)
			{
				if(--Interval_Time==0)Interval_Time=10;
			}
		break;
		case 9:
			if(Seg_Show_Mode==1)
			{
				Humidity_Parameter+=5;
				if(Humidity_Parameter==95)Humidity_Parameter=30;
			}
			else if(Seg_Show_Mode==2)
			{
				if(++Interval_Time==11)Interval_Time=1;
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	ADC_Rb2_Voltage=AD_Read(0x03)/51.0;
	if(ADC_Rb2_Voltage>=1 && ADC_Rb2_Voltage<=4)
		Humidity=(ADC_Rb2_Voltage*80-50)/3;
	else if(ADC_Rb2_Voltage<1)
		Humidity=10;
	else if(ADC_Rb2_Voltage>4)
		Humidity=90;
	
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;
			Seg_Buf[6]=Humidity<=9?10:Humidity/10%10;
			Seg_Buf[7]=Humidity/1%10;
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[6]=Humidity_Parameter<=9?10:Humidity_Parameter/10%10;
			Seg_Buf[7]=Humidity_Parameter/1%10;
		break;
		case 2:
			Seg_Buf[0]=13;
			Seg_Buf[6]=Interval_Time<=9?10:Interval_Time/10%10;
			Seg_Buf[7]=Interval_Time/1%10;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Seg_Show_Mode==2?1:0;
	
	Led_Buf[7]=Relay_ON_OFF_Mode==1?1:0;
	
	if(Relay_ON_OFF_Mode==1)
	{
		if(Humidity<Humidity_Parameter && Relay_Keep_Flag==0)
		{	
			Relay_Buf=1;
			Relay_Keep_Flag=1;
		}
		else if(Humidity>=Humidity_Parameter)
		{
			Relay_Buf=0;
			Relay_Keep_Flag=0;
		}
	}
	else
	{
		Time_Count=0;
		Relay_Buf=0;
		Relay_Keep_Flag=0;
	}
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
	
	if(Relay_Buf==1)
	{
		if(++Time_Count==Interval_Time*1000)
		{
			Time_Count=0;
			Relay_Buf=0;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	
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
