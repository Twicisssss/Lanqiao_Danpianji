#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <onewire.H>
#include <iic.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};

unsigned char Seg_Show_Mode=0;//【0-时间】【1-数据】【2-查询】
bit L_T_Get_Mode=0;//【0-自动】【1-手动】

unsigned char Rtc[3]={23,59,57,};
float ADC_Light_Voltage;
float Temperature;
float ADC_Light_Voltage_Save;
float Temperature_Save;

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
		if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
	}
	else if(Key_Down==8)//手动采集按键，按下S8按键可以手动采集光照与温度数值。
	{
		L_T_Get_Mode=1;
	}
}
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	RTC_Read(Rtc);
	Temperature=Temperature_Read();
	ADC_Light_Voltage=AD_Read(0x01)/51.0;
	
	if(L_T_Get_Mode==0)
	{
		if(Rtc[2]==0)
		{
			Temperature_Save=Temperature;
			ADC_Light_Voltage_Save=ADC_Light_Voltage;
		}
	}
	else
	{
		Temperature_Save=Temperature;
		ADC_Light_Voltage_Save=ADC_Light_Voltage;
		L_T_Get_Mode=0;
	}
	
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=Rtc[0]/10%10;
			Seg_Buf[1]=Rtc[0]%10;
			Seg_Buf[2]=11;
			Seg_Buf[3]=Rtc[1]/10%10;
			Seg_Buf[4]=Rtc[1]%10;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Rtc[2]/10%10;
			Seg_Buf[7]=Rtc[2]%10;
		break;
		case 1:
			;
			Seg_Buf[0]=(unsigned char)(ADC_Light_Voltage)+',';
			Seg_Buf[1]=(unsigned char)(ADC_Light_Voltage*10)%10;
			Seg_Buf[2]=(unsigned int)(ADC_Light_Voltage*100)%10;
			Seg_Buf[3]=10;
			Seg_Buf[4]=Temperature>=10?(unsigned char)(Temperature)/10%10:10;
			Seg_Buf[5]=(unsigned char)(Temperature)/1%10+',';
			Seg_Buf[6]=(unsigned int)(Temperature*10)%10;
			Seg_Buf[7]=(unsigned int)(Temperature*100)%10;
		break;
		case 2:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
			if(ADC_Light_Voltage_Save!=0)
			{
				Seg_Buf[2]=(unsigned char)(ADC_Light_Voltage_Save)+',';
				Seg_Buf[3]=(unsigned char)(ADC_Light_Voltage_Save*10)%10;
			}

			if(Temperature_Save!=0)
			{
				Seg_Buf[5]=Temperature>=10?(unsigned char)(Temperature_Save)/10%10:10;
				Seg_Buf[6]=(unsigned char)(Temperature_Save)/1%10+',';
				Seg_Buf[7]=(unsigned int)(Temperature_Save*10)%10;
			}
		break;
	}
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
	RTC_Set(Rtc);
	while(Temperature_Read()==85);
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
	}
}


