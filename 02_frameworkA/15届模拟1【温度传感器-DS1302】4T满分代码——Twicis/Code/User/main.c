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
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

idata unsigned char Seg_Show_Mode=0;//【0-时间】【1-温度】【2-定时】

idata unsigned char Rtc[3]={0x23,0x08,0x59};
idata unsigned char Rtc_Parameter[2][3]={{0x11,0x30,0x00},{0x11,0x30,0x00}};//【0-参数值】【1-设置值】

idata float Temperature;
idata bit T_ON_OFF_Moed=0;

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
			if(Seg_Show_Mode==1)
			{
				Rtc_Parameter[1][0]=Rtc_Parameter[0][0];
				Rtc_Parameter[1][1]=Rtc_Parameter[0][1];
				Rtc_Parameter[1][2]=Rtc_Parameter[0][2];
			}
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5:
			if(Seg_Show_Mode==0)
			{
				Rtc[0]++;
				if(Rtc[0]%16==0x0a)
					Rtc[0]+=0x06;
				if(Rtc[0]==0x24)
					Rtc[0]=0x00;
				RTC_Set(Rtc);
			}
			else if(Seg_Show_Mode==2)
			{
				Rtc_Parameter[1][0]++;
				if(Rtc_Parameter[1][0]%16==0x0a)
					Rtc_Parameter[1][0]+=0x06;
				if(Rtc_Parameter[1][0]==0x25)
					Rtc_Parameter[1][0]=0x00;
			}
		break;
		case 8:
			if(Seg_Show_Mode==0)
			{
				Rtc[1]++;
				if(Rtc[1]%16==0x0a)
					Rtc[1]+=0x06;
				if(Rtc[1]==0x60)
					Rtc[1]=0x00;
				RTC_Set(Rtc);
			}
			else if(Seg_Show_Mode==2)
			{
				Rtc_Parameter[1][1]++;
				if(Rtc_Parameter[1][1]%16==0x0a)
					Rtc_Parameter[1][1]+=0x06;
				if(Rtc_Parameter[1][1]==0x60)
					Rtc_Parameter[1][1]=0x00;
			}
		break;
		case 9:
			if(Seg_Show_Mode==0)
			{
				Rtc[2]=0x00;
				RTC_Set(Rtc);
			}
			else if(Seg_Show_Mode==2)
			{
				Rtc_Parameter[0][0]=Rtc_Parameter[1][0];
				Rtc_Parameter[0][1]=Rtc_Parameter[1][1];
				Rtc_Parameter[0][2]=Rtc_Parameter[1][2];
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	RTC_Read(Rtc);
	if(Rtc[0]==Rtc_Parameter[0][0] && Rtc[1]==Rtc_Parameter[0][1])
		T_ON_OFF_Moed=1;
	else
		T_ON_OFF_Moed=0;
	if(T_ON_OFF_Moed==1)
		Temperature=Temperature_Read();
	else
		Temperature=0;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=Rtc[0]/16%16;
			Seg_Buf[1]=Rtc[0]%16;
			Seg_Buf[2]=11;
			Seg_Buf[3]=Rtc[1]/16%16;
			Seg_Buf[4]=Rtc[1]%16;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Rtc[2]/16%16;
			Seg_Buf[7]=Rtc[2]%16;
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=Temperature==0?11:(unsigned char)(Temperature)/10%10;
			Seg_Buf[6]=Temperature==0?11:(unsigned char)(Temperature)/1%10;
			Seg_Point[6]=Temperature==0?0:1;
			Seg_Buf[7]=Temperature==0?11:(unsigned int)(Temperature*10)%10;
		break;
		case 2:
			Seg_Point[6]=0;
			Seg_Buf[0]=13;
			Seg_Buf[1]=Seg_Buf[2]=10;
			Seg_Buf[3]=Rtc_Parameter[1][0]/16%16;
			Seg_Buf[4]=Rtc_Parameter[1][0]%16;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Rtc_Parameter[1][1]/16%16;
			Seg_Buf[7]=Rtc_Parameter[1][1]%16;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Seg_Show_Mode==2?1:0;
	
	Led_Buf[7]=T_ON_OFF_Moed==1?Time_100ms_Flag:0;
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
	
	if(T_ON_OFF_Moed==1)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			Time_100ms_Flag^=1;
		}
	}
	else
		Time_100ms=Time_100ms_Flag=0;
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	RTC_Set(Rtc);
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
