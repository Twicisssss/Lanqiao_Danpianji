/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <iic.H>

/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0;
idata unsigned char Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-频率】【1-参数】【2-时间】【3-回显】
idata unsigned char Select_Index=0;//【0-超限参数/频率回显】【1-校准值/时间回显】
idata unsigned char Rtc[3]={0x13,0x03,0x05};

idata unsigned int Time_1s;
idata long int Ne555_F;
idata long int Frequency;
idata unsigned int Frequency_Over_Parameter=2000;//超限参数可调整范围：1000Hz～9000Hz
idata int Frequency_Calibration=0;//校准值参数可调整范围：-900Hz～900Hz
idata int Frequency_Max;
idata unsigned int F_Max_Time[3];

idata float F_V_DAC_Data;

idata unsigned char Time_200ms;
idata bit Time_200ms_Flag;
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
			if(++Seg_Show_Mode==4)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1 || Seg_Show_Mode==3)Select_Index=0;
		break;
		case 5:
			if(++Select_Index==2)Select_Index=0;
		break;
		case 8:
			if(Seg_Show_Mode==1)
			{
				if(Select_Index==0)
				{
					Frequency_Over_Parameter+=1000;
					if(Frequency_Over_Parameter==10000)Frequency_Over_Parameter=9000;
				}
				else
				{
					Frequency_Calibration+=100;
					if(Frequency_Calibration==1000)Frequency_Calibration=900;
				}
			}
		break;
		case 9:
			if(Seg_Show_Mode==1)
			{
				if(Select_Index==0)
				{
					Frequency_Over_Parameter-=1000;
					if(Frequency_Over_Parameter==0)Frequency_Over_Parameter=1000;
				}
				else
				{
					Frequency_Calibration-=100;
					if(Frequency_Calibration==-1000)Frequency_Calibration=-900;
				}
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	RTC_Read(Rtc);
	
	Frequency=Ne555_F+Frequency_Calibration;
	if(Frequency_Max<Frequency)
	{
		Frequency_Max=Frequency;
		F_Max_Time[0]=Rtc[0];
		F_Max_Time[1]=Rtc[1];
		F_Max_Time[2]=Rtc[2];
	}

	if(Frequency>=0)
	{
		if(Frequency<Frequency_Over_Parameter && Frequency>500)
			F_V_DAC_Data=((((float)Frequency-500)*4)/((float)Frequency_Over_Parameter-500))+1.0;
		else if(Frequency<=500)
			F_V_DAC_Data=1.0;
		else if(Frequency>=Frequency_Over_Parameter)
			F_V_DAC_Data=5.0;
	}
	else
		F_V_DAC_Data=0;
	DA_Write(F_V_DAC_Data*51);
	
	
	switch(Seg_Show_Mode)
	{
		case 0://【0-频率】
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=10;
			if(Frequency>=0)
			{
				Seg_Buf[3]=Frequency/10000%10;
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
				for(i=3;i<=7;i++)
				{
					if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
						Seg_Buf[i]=10;
				}
			}
			else
			{
				Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
				Seg_Buf[6]=Seg_Buf[7]=16;
			}
		break;
		case 1://【1-参数】Frequency_Over_Parameter
			Seg_Buf[0]=13;
			Seg_Buf[1]=Select_Index+1;
			Seg_Buf[2]=Seg_Buf[3]=10;
			if(Select_Index==0)//【0-超限参数】
			{
				Seg_Buf[4]=Frequency_Over_Parameter/1000%10;
				Seg_Buf[5]=Frequency_Over_Parameter/100%10;
				Seg_Buf[6]=Frequency_Over_Parameter/10%10;
				Seg_Buf[7]=Frequency_Over_Parameter/1%10;
			}
			else//【1-校准值】
			{
				if(Frequency_Calibration>=0)
				{
					Seg_Buf[4]=Frequency_Calibration/1000%10;
					Seg_Buf[5]=Frequency_Calibration/100%10;
					Seg_Buf[6]=Frequency_Calibration/10%10;
					Seg_Buf[7]=Frequency_Calibration/1%10;
					for(i=4;i<=6;i++)
					{
						if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
							Seg_Buf[i]=10;
					}
				}
				else
				{
					Seg_Buf[4]=11;
					Seg_Buf[5]=-Frequency_Calibration/100%10;
					Seg_Buf[6]=-Frequency_Calibration/10%10;
					Seg_Buf[7]=-Frequency_Calibration/1%10;
				}
			}
		break;
		case 2://【2-时间】
			Seg_Buf[0]=Rtc[0]/16%16;
			Seg_Buf[1]=Rtc[0]%16;
			Seg_Buf[2]=11;
			Seg_Buf[3]=Rtc[1]/16%16;
			Seg_Buf[4]=Rtc[1]%16;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Rtc[2]/16%16;
			Seg_Buf[7]=Rtc[2]%16;
		break;
		case 3://【3-回显】
			Seg_Buf[0]=14;
			if(Select_Index==0)//【0-频率回显】
			{
				Seg_Buf[1]=12;
				Seg_Buf[2]=10;
				
				Seg_Buf[3]=Frequency_Max/10000%10;
				Seg_Buf[4]=Frequency_Max/1000%10;
				Seg_Buf[5]=Frequency_Max/100%10;
				Seg_Buf[6]=Frequency_Max/10%10;
				Seg_Buf[7]=Frequency_Max/1%10;
				for(i=3;i<=7;i++)
				{
					if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
						Seg_Buf[i]=10;
				}
			}
			else//【1-时间回显】
			{
				Seg_Buf[1]=15;
				
				Seg_Buf[2]=F_Max_Time[0]/16%16;
				Seg_Buf[3]=F_Max_Time[0]%16;
				Seg_Buf[4]=F_Max_Time[1]/16%16;
				Seg_Buf[5]=F_Max_Time[1]%16;
				Seg_Buf[6]=F_Max_Time[2]/16%16;
				Seg_Buf[7]=F_Max_Time[2]%16;
			}
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?Time_200ms_Flag:0;
	if(Frequency>=0)
		Led_Buf[1]=Frequency>Frequency_Over_Parameter?Time_200ms_Flag:0;
	else
		Led_Buf[1]=1;
}
/*定时器0区域*/
void Timer0_Init(void)		//0毫秒@12.000MHz
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
	if(++Seg_Slow==80)Seg_Slow=0;
	
	
	if(++Time_1s==1000)
	{
		TR0 = 0;
		Time_1s=0;
		Ne555_F=(TH0<<8)|TL0;
		TH0=TL0=0;
		TR0 = 1;
	}
	
	
	if(++Time_200ms==200)
	{
		Time_200ms=0;
		Time_200ms_Flag^=1;
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
	
	Timer1_Init();
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
