/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <onewire.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
unsigned char Relay_Buf=0;


unsigned char Seg_Show_Mode=0;//【0-温度显示】、【1-时间显示】、【2-参数设置】
unsigned char U2_Time_Mode=0;//【0-时分】、【1-分秒】
unsigned char Realy_Work_Mode=0;//继电器工作模式【0-温度控制】【1-时间控制】

float Temperature;//实时温度
float Temperature_Parameter=23.0;//温度参数
unsigned Temperature_Parameter_Set=23;//温度参数设置值
unsigned char Rtc[3]={0x23,0x59,0x55};

bit Each_Hour_Flag=0;
bit L3_Flash_Flag=0;

unsigned int Time_5000ms;
unsigned int Time_100ms;
/*Key控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Seg_Show_Mode==1)
	{
		if(Key_Old==17)
			{U2_Time_Mode=1;}
		else
			{U2_Time_Mode=0;}
	}

	switch(Key_Down)
	{
		case 12://S12：“切换”按键，按下切换【0-温度显示】、【1-时间显示】、【2-参数设置】界面
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==2)//进入【2-参数设置】界面
			{
				Temperature_Parameter_Set=(unsigned char)Temperature_Parameter;
			}
			if(Seg_Show_Mode==0)//进入【0-温度显示】界面，离开【2-参数设置】界面
			{
				Temperature_Parameter=(float)Temperature_Parameter_Set;
			}
		break;
		case 13://S13：“模式”按键，按下切换继电器【0-温度控制】【1-时间控制】工作模式。
			if(++Realy_Work_Mode==2)Realy_Work_Mode=0;
			Led_Buf[2]=0;
			Relay_Buf=0;
		break;
		case 16://S16：“加”按键，在【2-参数设置】界面下按下，温度参数增加1℃。
			if(Seg_Show_Mode==2)
			{
				if(++Temperature_Parameter_Set==100)Temperature_Parameter_Set=99;
			}
		break;
		case 17://S17：“减”按键，在【2-参数设置】界面下按下，温度参数减少1℃。
			if(Seg_Show_Mode==2)
			{
				if(--Temperature_Parameter_Set==9)Temperature_Parameter_Set=10;
			}
		break;
	}
}
/*Seg显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read();
	Rtc_Read(Rtc);
	
	if(U2_Time_Mode==0)
	{
		switch(Seg_Show_Mode)
		{
			case 0://【0-温度显示】
				Seg_Buf[0]=11;
				Seg_Buf[1]=1;//U1
			Seg_Buf[2]=10;Seg_Buf[3]=10;Seg_Buf[4]=10;
				Seg_Buf[5]=(unsigned int)(Temperature/10)%10;
				Seg_Buf[6]=(unsigned int)(Temperature/1)%10;
			Seg_Point[6]=1;
				Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
			break;
			case 1://【1-时间显示】
			Seg_Point[6]=0;
				Seg_Buf[0]=11;
				Seg_Buf[1]=2;//U2
			Seg_Buf[2]=10;
				Seg_Buf[3]=Rtc[0]/16;
				Seg_Buf[4]=Rtc[0]%16;
				Seg_Buf[5]=12;
				Seg_Buf[6]=Rtc[1]/16;
				Seg_Buf[7]=Rtc[1]%16;
			break;
			case 2://【2-参数设置】
				Seg_Buf[0]=11;
				Seg_Buf[1]=3;//U3
			Seg_Buf[2]=10;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Buf[5]=10;
				Seg_Buf[6]=Temperature_Parameter_Set/10%10;
				Seg_Buf[7]=Temperature_Parameter_Set/1%10;
			break;
		}
	}
	else if(U2_Time_Mode==1)
	{
		Seg_Point[6]=0;
		Seg_Buf[0]=11;
		Seg_Buf[1]=2;//U2
		Seg_Buf[2]=10;
		Seg_Buf[3]=Rtc[1]/16;
		Seg_Buf[4]=Rtc[1]%16;
		Seg_Buf[5]=12;
		Seg_Buf[6]=Rtc[2]/16;
		Seg_Buf[7]=Rtc[2]%16;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	if(Rtc[1]==0x00 && Rtc[2]==0x00 && Each_Hour_Flag==0)
		{Each_Hour_Flag=1;}
		
	Led_Buf[0]=Each_Hour_Flag;
	Led_Buf[1]=!Realy_Work_Mode;
		
	Led_Buf[2]=Relay_Buf?L3_Flash_Flag:0;
		
		
	if(Realy_Work_Mode==0)//继电器【0-温度控制】
	{
		Relay_Buf=(Temperature>Temperature_Parameter);
	}
	else//继电器【1-时间控制】
	{
		Relay_Buf=Each_Hour_Flag;
	}

	
}
/*定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA=1;
}
void Timer0_Isr(void) interrupt 1
{
	if(++Key_Slow==20)Key_Slow=0;
	if(++Seg_Slow==80)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Relay(Relay_Buf);
	
	
	if(Each_Hour_Flag==1)
	{
		if(++Time_5000ms==5000)
		{
			Time_5000ms=0;
			Each_Hour_Flag=0;
		}
	}
	else
		{Time_5000ms=0;}
	
	
	if(Relay_Buf==1)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			L3_Flash_Flag^=1;
		}
	}
	else
		{Time_100ms=0;}
}
/*延时函数*/
void Delay850ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 39;
	j = 195;
	k = 2;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Rtc_Set(Rtc);
	Temperature=Temperature_Read();
	Delay850ms();
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
