/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <Ultrasound.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i;

idata unsigned char Seg_Show_Mode=0;//【0-测距】【1-参数】【2-记录】
idata unsigned char Parameter_Mode=0;//【0-按键模式】【1-旋钮模式】
idata unsigned char US_Distance;
idata unsigned char Distance_Parameter[2]={60,10};//【0-上限参数】【1-下限参数】
idata bit Knob_Parameter_Index;
idata unsigned char Alarms_Count;
idata bit Alarm_Flag;

idata float ADC_Rb2_Voltage;
idata unsigned char Voltage_Equal_Value;

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
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5:
			if(Seg_Show_Mode==1)
			{
				if(++Parameter_Mode==2)Parameter_Mode=0;
			}
			else if(Seg_Show_Mode==2)
			{
				Alarms_Count=0;
			}
		break;
		case 9://上限
			if(Seg_Show_Mode==1)//【1-参数】
			{
				if(Parameter_Mode==0)//【0-按键模式】
				{
					Distance_Parameter[0]+=10;
					if(Distance_Parameter[0]==100)Distance_Parameter[0]=50;
				}
				else//【1-旋钮模式】
				{
					Knob_Parameter_Index=0;
				}
			}
		break;
		case 8://下限
			if(Seg_Show_Mode==1)//【1-参数】
			{
				if(Parameter_Mode==0)//【0-按键模式】
				{
					Distance_Parameter[1]+=10;
					if(Distance_Parameter[1]==50)Distance_Parameter[1]=0;
				}
				else//【1-旋钮模式】
				{
					Knob_Parameter_Index=1;
				}
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	US_Distance=US_Distance_Get();
	if(Alarm_Flag==0)
	{
		if(US_Distance>Distance_Parameter[0] || US_Distance<Distance_Parameter[1])
		{
			Alarm_Flag=1;
			Alarms_Count++;
		}
	}
	else
	{
		if(US_Distance>=Distance_Parameter[1] && US_Distance<=Distance_Parameter[0])
			Alarm_Flag=0;
	}
	
	
	ADC_Rb2_Voltage=AD_Read(0x03)/51.0;
	if(Parameter_Mode==1)
	{
		if(Knob_Parameter_Index==0)
		{
//			if(ADC_Rb2_Voltage>=0.0 && ADC_Rb2_Voltage<1.0)
//				Distance_Parameter[0]=50;
//			else if(ADC_Rb2_Voltage>=1.0 && ADC_Rb2_Voltage<2.0)
//				Distance_Parameter[0]=60;
//			else if(ADC_Rb2_Voltage>=2.0 && ADC_Rb2_Voltage<3.0)
//				Distance_Parameter[0]=70;
//			else if(ADC_Rb2_Voltage>=3.0 && ADC_Rb2_Voltage<4.0)
//				Distance_Parameter[0]=80;
//			else if(ADC_Rb2_Voltage>=4.0 && ADC_Rb2_Voltage<=5.0)
//				Distance_Parameter[0]=90;
			Voltage_Equal_Value=(unsigned char)ADC_Rb2_Voltage * 10;
			if (Voltage_Equal_Value >= 50)
				Voltage_Equal_Value = 40;
			Distance_Parameter[0] = 50 + Voltage_Equal_Value;
		}
		else
		{
			Voltage_Equal_Value=(unsigned char)ADC_Rb2_Voltage * 10;
			if (Voltage_Equal_Value >= 50)
				Voltage_Equal_Value = 40;
			Distance_Parameter[1] = 0 + Voltage_Equal_Value;
		}
	}
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=US_Distance/100%10;
			Seg_Buf[6]=US_Distance/10%10;
			Seg_Buf[7]=US_Distance/1%10;
			for(i=5;i<7;i++)
			{
				if(Seg_Buf[i]!=0)break;
				Seg_Buf[i]=10;
			}
		break;
		case 1:
			Seg_Buf[0]=13;
			Seg_Buf[1]=Parameter_Mode+1;
			Seg_Buf[2]=10;
			Seg_Buf[3]=Distance_Parameter[1]/10%10;
			Seg_Buf[4]=Distance_Parameter[1]/1%10;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Distance_Parameter[0]/10%10;
			Seg_Buf[7]=Distance_Parameter[0]/1%10;
		break;
		case 2:
			Seg_Buf[0]=14;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=10;
		if(Alarms_Count<10)
			Seg_Buf[7]=Alarms_Count;
		else
			Seg_Buf[7]=11;
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
	
	Led_Buf[7]=Alarm_Flag==0?1:Time_100ms_Flag;;
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
	
	if(++Time_100ms==100)
	{
		Time_100ms=0;
		Time_100ms_Flag^=1;
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
