/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <onewire.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata bit Beeper_Buf=0;
idata bit Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-温度】【1-时间】【2-参数】
idata unsigned char Work_Mode=0;//【0-温度控制】【1-时间控制】
idata unsigned char Rtc_Show_Mode=0;//【0-时分】【1-分秒】

idata unsigned char Rtc[3]={0x23,0x58,0x55};//【0-时】【1-分】【2-秒】
idata float Temperature;
idata float Temperature_Parameter[2]={23,23};//【0-参数实际值】【1-参数修改值】

idata unsigned int Relay_Time_5000ms;
idata unsigned int Led_Time_5000ms;
idata unsigned int Led_Time_100ms;
idata bit Led_Flash_Flag;
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
		case 12:
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
//			if(Seg_Show_Mode==2)
//			{
//				Temperature_Parameter[1]=Temperature_Parameter[0];
//			}
//			else if(Seg_Show_Mode==0)
//			{
//				Temperature_Parameter[0]=Temperature_Parameter[1];
//			}
		break;
		case 13:
			if(++Work_Mode==2)Work_Mode=0;
		break;
		case 16:
			if(Seg_Show_Mode==2)
			{
				if(++Temperature_Parameter[0]==100)Temperature_Parameter[0]=99;
			}
		break;
		case 17:
			if(Seg_Show_Mode==2)
			{
				if(--Temperature_Parameter[0]==9)Temperature_Parameter[0]=10;
			}
		break;
	}
	if(Seg_Show_Mode==1)
	{
		if(Key_Old==17)
			Rtc_Show_Mode=1;
		else
			Rtc_Show_Mode=0;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
//	switch(Seg_Slow)
//	{
//		case 30:
//			Temperature=Temperature_Read();
//		break;
//		case 60:
//			
//		break;
//		case 80:
//			RTC_Read(Rtc);
//		break;
//	}
	
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read();
	RTC_Read(Rtc);

	Seg_Buf[0]=12;
	Seg_Buf[1]=Seg_Show_Mode+1;
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[6]=(unsigned char)(Temperature)/1%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
		break;
		case 1:
			Seg_Point[6]=0;
			Seg_Buf[2]=10;
			Seg_Buf[5]=11;
			if(Rtc_Show_Mode==0)
			{
				Seg_Buf[3]=Rtc[0]/16%16;
				Seg_Buf[4]=Rtc[0]%16;
				Seg_Buf[6]=Rtc[1]/16%16;
				Seg_Buf[7]=Rtc[1]%16;
			}
			else
			{
				Seg_Buf[3]=Rtc[1]/16%16;
				Seg_Buf[4]=Rtc[1]%16;
				Seg_Buf[6]=Rtc[2]/16%16;
				Seg_Buf[7]=Rtc[2]%16;
			}
		break;
		case 2:
			Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=(unsigned char)(Temperature_Parameter[0])/10%10;
			Seg_Buf[7]=(unsigned char)(Temperature_Parameter[0])/1%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Beeper_Buf=0;
	if(Work_Mode==0)
	{
		if(Temperature>Temperature_Parameter[0])
			Relay_Buf=1;
		else
			Relay_Buf=0;
		Relay(Relay_Buf);
	}
	else
	{
		if(Rtc[1]==0x00 && Rtc[2]==0x00 && Relay_Buf==0)
			Relay_Buf=1;
		Relay(Relay_Buf);
	}
	
	if(Rtc[1]==0x00 && Rtc[2]==0x00 && Led_Buf[0]==0)
		Led_Buf[0]=1;
	
	Led_Buf[1]=(Work_Mode==0)?1:0;
	
	if(Relay_Buf==1)
		Led_Buf[2]=(Led_Flash_Flag)?1:0;
	else
		Led_Buf[2]=0;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	
	if(Relay_Buf==1)
	{
		if(++Relay_Time_5000ms==5000)
		{
			Relay_Time_5000ms=0;
			Relay_Buf=0;
		}
	}
	
	if(Led_Buf[0]==1)
	{
		if(++Led_Time_5000ms==5000)
		{
			Led_Time_5000ms=0;
			Led_Buf[0]=0;
		}
	}
	else
		Led_Buf[0]=Led_Time_5000ms=0;
	
	if(++Led_Time_100ms==100)
	{
		Led_Time_100ms=0;
		Led_Flash_Flag^=1;
	}
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
//	Relay(Relay_Buf);
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	RTC_Set(Rtc);
	while(Temperature_Read()==85);
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
