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
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i,j;


idata unsigned char Seg_Show_Mode=0;//【0-时间】【1-数据】【2-历史】
idata unsigned char History_Index=0;

idata unsigned char Rtc[3]={0x23,0x59,0x50};
idata unsigned char Rtc_Trigger[3][3];

idata float ADC_Light_Voltage;
idata float ADC_Rb2_Voltage;
idata bit Trigger_Flag;//【*-触发】
idata bit Trigger_Keep;
idata unsigned int Time_3000ms;
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
			if(Seg_Show_Mode==1)History_Index=0;
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5:
			if(Seg_Show_Mode==2)
			{
				if(++History_Index==3)History_Index=0;
			}
		break;
		case 8:
			if(Seg_Show_Mode==2)
			{
				for(i=0;i<3;i++)
				{
					for(j=0;j<3;j++)
						Rtc_Trigger[i][j]=0;
				}
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	switch(Seg_Slow)
	{
		case 30:
			ADC_Light_Voltage=AD_Read(0x03)/51.0;
		break;
		case 60:
			ADC_Rb2_Voltage=AD_Read(0x01)/51.0;
		break;
		case 90:
			RTC_Read(Rtc);
		break;
	}
	
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	if(ADC_Light_Voltage<ADC_Rb2_Voltage)
	{
		if(Trigger_Keep==0)
		{
			Trigger_Flag=1;
			Rtc_Trigger[2][0]=Rtc_Trigger[1][0];
			Rtc_Trigger[2][1]=Rtc_Trigger[1][1];
			Rtc_Trigger[2][2]=Rtc_Trigger[1][2];
			Rtc_Trigger[1][0]=Rtc_Trigger[0][0];
			Rtc_Trigger[1][1]=Rtc_Trigger[0][1];
			Rtc_Trigger[1][2]=Rtc_Trigger[0][2];
			Rtc_Trigger[0][0]=Rtc[0];
			Rtc_Trigger[0][1]=Rtc[1];
			Rtc_Trigger[0][2]=Rtc[2];
		}
		Trigger_Keep=1;
	}
	else
		Trigger_Keep=0;
	
	
	if(Trigger_Flag==0)
	{
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
				Seg_Buf[1]=(unsigned char)(ADC_Light_Voltage)%10;
				Seg_Point[1]=1;
				Seg_Buf[2]=(unsigned int)(ADC_Light_Voltage*10)%10;
				Seg_Buf[3]=(unsigned int)(ADC_Light_Voltage*100)%10;
			
				Seg_Buf[4]=13;
				Seg_Buf[5]=(unsigned char)(ADC_Rb2_Voltage)%10;
				Seg_Point[5]=1;
				Seg_Buf[6]=(unsigned int)(ADC_Rb2_Voltage*10)%10;
				Seg_Buf[7]=(unsigned int)(ADC_Rb2_Voltage*100)%10;
			break;
			case 2:
				Seg_Point[1]=Seg_Point[5]=0;
				Seg_Buf[0]=14;
				Seg_Buf[1]=History_Index+1;
				Seg_Buf[2]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][0]/16%16;
				Seg_Buf[3]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][0]%16;
				Seg_Buf[4]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][1]/16%16;
				Seg_Buf[5]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][1]%16;
				Seg_Buf[6]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][2]/16%16;
				Seg_Buf[7]=(Rtc_Trigger[History_Index][0]==0 && Rtc_Trigger[History_Index][1]==0 && Rtc_Trigger[History_Index][2]==0)?11:Rtc_Trigger[History_Index][2]%16;
			break;
		}
	}
	else//触发记录功能
	{
		Seg_Point[1]=Seg_Point[5]=0;
		Seg_Buf[0]=Seg_Buf[1]=15;
		Seg_Buf[2]=Rtc_Trigger[0][0]/16%16;
		Seg_Buf[3]=Rtc_Trigger[0][0]%16;
		Seg_Buf[4]=Rtc_Trigger[0][1]/16%16;
		Seg_Buf[5]=Rtc_Trigger[0][1]%16;
		Seg_Buf[6]=Rtc_Trigger[0][2]/16%16;
		Seg_Buf[7]=Rtc_Trigger[0][2]%16;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=(Seg_Show_Mode==0 && Trigger_Flag==0)?1:0;
	Led_Buf[1]=(Seg_Show_Mode==1 && Trigger_Flag==0)?1:0;
	Led_Buf[2]=(Seg_Show_Mode==2 && Trigger_Flag==0)?1:0;
	Led_Buf[7]=Trigger_Flag==1?1:0;
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
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
	
	if(Trigger_Flag==1)
	{
		if(++Time_3000ms==3000)
		{
			Time_3000ms=0;
			Trigger_Flag=0;
		}
	}
	else
		Time_3000ms=0;
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
