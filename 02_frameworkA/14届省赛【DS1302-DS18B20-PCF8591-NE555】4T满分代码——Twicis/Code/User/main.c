/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <onewire.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata bit Beeper_Buf=0;
idata bit Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-时间界面】【1-回显界面】【2-参数界面】
idata unsigned char Echo_Show_Mode=0;//【0-温度回显】【1-湿度回显】【2-时间回显】
idata unsigned char T_H_Get_Mode=0;//【0-关闭温湿度采集】【1-开启温湿度采集】
idata unsigned int Time_3000ms;


idata unsigned char Rtc[3]={0x13,0x03,0x05};

idata unsigned int Time_1s;
idata unsigned int Frequency;
idata float Temperature_Parameter=30;
idata float Temperature;
idata float Humidity;

idata float ADC_Light;
idata bit Bright_State;//【0-挡光 暗】【1-未挡 亮】
idata bit Bright_State_Old;//【0-挡光 暗】【1-未挡 亮】

idata float Temperature_Max;
idata float Temperature_Average;
idata float Humidity_Max;
idata float Humidity_Average;
idata unsigned char Rtc_Echo[3];
idata unsigned char Count=0;
idata float Temperature_Old;
idata float Humidity_Old;
idata bit Humidity_Error;

idata bit Key_Press_Flag;
idata unsigned int Time_2000ms;

idata unsigned char Time_100ms;
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
		case 4:
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)Echo_Show_Mode=0;
		break;
		case 5:
			if(Seg_Show_Mode==1)
			{
				if(++Echo_Show_Mode==3)Echo_Show_Mode=0;
			}
		break;
		case 8:
			if(Seg_Show_Mode==2)
			{
				if(++Temperature_Parameter==100)Temperature_Parameter=99;
			}
		break;
		case 9:
			if(Seg_Show_Mode==2)
			{
				if(--Temperature_Parameter==-1)Temperature_Parameter=0;
			}
		break;
	}
	if(Seg_Show_Mode==1 && Echo_Show_Mode==2)
	{
		if(Key_Down==9)
		{
			Key_Press_Flag=1;
		}
		if(Key_Up==9)
		{
			if(Time_2000ms==2000)
			{
				Time_2000ms=0;
				Temperature_Old=Temperature_Max=Temperature_Average=0;
				Humidity_Old=Humidity_Max=Humidity_Average=0;
				Rtc_Echo[0]=Rtc_Echo[1]=Rtc_Echo[2]=0x00;;
				Count=0;
			}
			else
			{
				Time_2000ms=0;
			}
			Key_Press_Flag=0;
		}
	}
	
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	RTC_Read(Rtc);
	
	
	Bright_State_Old=Bright_State;
	ADC_Light=AD_Read(1)/51.0;
	if(ADC_Light<1.0)
		Bright_State=0;
	else
		Bright_State=1;
	if(T_H_Get_Mode==0 && Bright_State==0 && Bright_State_Old==1)
	{
		T_H_Get_Mode=1;
		Temperature_Old=Temperature;
		Humidity_Old=Humidity;
		
		Temperature=Temperature_Read();
		if(Frequency>200 && Frequency<2000)
			Humidity=(2*Frequency+50)/45.0;
		else if(Frequency<=200)
			Humidity=0;
		else if(Frequency>=2000)
			Humidity=0;
		
		if(Humidity!=0)
		{
			Humidity_Error=0;
			if(Temperature>Temperature_Max)
				Temperature_Max=Temperature;
			if(Humidity>Humidity_Max)
				Humidity_Max=Humidity;
			Temperature_Average=(Temperature_Average*Count+Temperature)/(Count+1);
			Humidity_Average=(Humidity_Average*Count+Humidity)/(Count+1);
			Rtc_Echo[0]=Rtc[0];
			Rtc_Echo[1]=Rtc[1];
			Rtc_Echo[2]=Rtc[2];
			Count++;
		}
		else
			Humidity_Error=1;
	}
	
	
	if(T_H_Get_Mode==0)
	{
		switch(Seg_Show_Mode)
		{
			case 0://【0-时间界面】
				Seg_Buf[0]=Rtc[0]/16%16;
				Seg_Buf[1]=Rtc[0]%16;
				Seg_Buf[2]=11;
				Seg_Buf[3]=Rtc[1]/16%16;
				Seg_Buf[4]=Rtc[1]%16;
				Seg_Buf[5]=11;
				Seg_Buf[6]=Rtc[2]/16%16;
				Seg_Buf[7]=Rtc[2]%16;
			break;
			case 1://【1-回显界面】
				if(Echo_Show_Mode==0)//【0-温度回显】
				{
					Seg_Buf[0]=12;
					Seg_Buf[1]=10;
					if(Count!=0)
					{
						Seg_Buf[2]=(unsigned char)(Temperature_Max)/10%10;
						Seg_Buf[3]=(unsigned char)(Temperature_Max)/1%10;
						Seg_Buf[4]=11;
						Seg_Buf[5]=(unsigned char)(Temperature_Average)/10%10;
						Seg_Buf[6]=(unsigned char)(Temperature_Average)/1%10;
						Seg_Point[6]=1;
						Seg_Buf[7]=(unsigned int)(Temperature_Average*10)%10;
					}
					else
					{
						Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
						Seg_Point[6]=0;
					}
				}
				else if(Echo_Show_Mode==1)//【1-湿度回显】
				{
					Seg_Buf[0]=13;
					Seg_Buf[1]=10;
					if(Count!=0)
					{
						Seg_Buf[2]=(unsigned char)(Humidity_Max)/10%10;
						Seg_Buf[3]=(unsigned char)(Humidity_Max)/1%10;
						Seg_Buf[4]=11;
						Seg_Buf[5]=(unsigned char)(Humidity_Average)/10%10;
						Seg_Buf[6]=(unsigned char)(Humidity_Average)/1%10;
						Seg_Point[6]=1;
						Seg_Buf[7]=(unsigned int)(Humidity_Average*10)%10;
					}
					else
					{
						Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
						Seg_Point[6]=0;
					}
				}
				else if(Echo_Show_Mode==2)//【2-时间回显】
				{
					Seg_Point[6]=0;
					Seg_Buf[0]=14;
					Seg_Buf[1]=Count/10%10;
					Seg_Buf[2]=Count/1%10;
					if(Count!=0)
					{
						Seg_Buf[3]=Rtc_Echo[0]/16%16;
						Seg_Buf[4]=Rtc_Echo[0]%16;
						Seg_Buf[5]=11;
						Seg_Buf[6]=Rtc_Echo[1]/16%16;
						Seg_Buf[7]=Rtc_Echo[1]%16;
					}
					else
					{
						Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
					}
				}
			break;
			case 2://【2-参数界面】
				Seg_Point[6]=0;
				Seg_Buf[0]=15;
				Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
				Seg_Buf[6]=(unsigned char)Temperature_Parameter/10%10;
				Seg_Buf[7]=(unsigned char)Temperature_Parameter/1%10;
			break;
		}
	}
	else
	{
		Seg_Point[6]=0;
		Seg_Buf[0]=16;
		Seg_Buf[1]=10;
		Seg_Buf[2]=10;
		Seg_Buf[3]=(unsigned char)(Temperature)/10%10;
		Seg_Buf[4]=(unsigned char)(Temperature)/1%10;
		Seg_Buf[5]=11;
		if(Humidity==0)
		{
			Seg_Buf[6]=Seg_Buf[7]=17;
		}
		else
		{
			Seg_Buf[6]=(unsigned char)(Humidity)/10%10;
			Seg_Buf[7]=(unsigned char)(Humidity)/1%10;
		}
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=(Seg_Show_Mode==0)&&(T_H_Get_Mode==0)?1:0;
	Led_Buf[1]=(Seg_Show_Mode==1)&&(T_H_Get_Mode==0)?1:0;
	Led_Buf[2]=(T_H_Get_Mode==1)?1:0;
	
	Led_Buf[3]=(Temperature>Temperature_Parameter)?Led_Flash_Flag:0;
	Led_Buf[4]=(Humidity_Error==1)?1:0;
	Led_Buf[5]=(Count>=2 && Humidity>Humidity_Old && Temperature>Temperature_Old)?1:0;
}
/*定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	
	if(++Time_1s==1000)
	{
		TR0 = 0;
		Time_1s=0;
		Frequency=(TH0<<8)|TL0;
		TH0=TL0=0;
		TR0 = 1;
	}
	
	
	if(T_H_Get_Mode==1)
	{
		if(++Time_3000ms==3000)
		{
			Time_3000ms=0;
			T_H_Get_Mode=0;
		}
	}
	
	
	if(Key_Press_Flag==1)
	{
		if(++Time_2000ms==2001)
		{
			Time_2000ms=2000;
		}
	}
	
	
	if(++Time_100ms==100)
	{
		Time_100ms=0;
		Led_Flash_Flag^=1;
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
	
	RTC_Set(Rtc);
	Temperature_Read();
	
	Timer1_Init();
	Timer0_Init();
}
/*主函数区域*/
void main()
{
	Init_Proc();
	while(1)
	{
		if(T_H_Get_Mode==0)
			Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}
