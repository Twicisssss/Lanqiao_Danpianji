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
idata unsigned char Realy_Buf=0;
idata unsigned char Beeper_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-时间】【1-输入】【2-记录】
idata unsigned char Rtc[3]={0x23,0x09,0x59};
idata unsigned char Rtc_Input_Start[2];

idata unsigned int Input_Data;
idata unsigned int Input_Data_Old;
idata unsigned char Key_Data_Location[10]={6,10,14,18,9,13,17,8,12,16};
idata bit Input_Success_Flag=0;

idata unsigned char EEPROM_Save_Data[4];
/*按键控制区域*/
void Key_Proc()
{
	unsigned char j;
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	
	if(Seg_Show_Mode==1)
	{
		for(j=0;j<10;j++)
		{
			if(Key_Down==Key_Data_Location[j] && Input_Data<1000 && !(j==0 && Input_Data==j))
			{
				if(Input_Data==0)
				{
					Rtc_Input_Start[0]=Rtc[0];
					Rtc_Input_Start[1]=Rtc[1];
				}
				Input_Data=Input_Data*10+j;
				Input_Success_Flag=1;
				break;
			}
		}
	}
	
	switch(Key_Down)
	{
		case 4:
			if(Seg_Show_Mode==1)
			{				
				Led_Buf[3] = (Input_Data > Input_Data_Old);

				EEPROM_Save_Data[0]=(Rtc_Input_Start[0]/16)*10+Rtc_Input_Start[0]%16;
				EEPROM_Save_Data[1]=(Rtc_Input_Start[1]/16)*10+Rtc_Input_Start[1]%16;
				EEPROM_Save_Data[2]=Input_Data>>8;
				EEPROM_Save_Data[3]=Input_Data&0xff;
				EEPROM_Write(EEPROM_Save_Data,0,4);
				
				Input_Data_Old = Input_Data;
				
				Input_Data=0;
				Input_Success_Flag=0;
			}
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5:
			if(Seg_Show_Mode==1)
			{
				Input_Data=0;
				Input_Success_Flag=0;
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
	
	
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=Rtc[0]/16%16;
			Seg_Buf[1]=Rtc[0]%16;
			Seg_Buf[2]=13;
			Seg_Buf[3]=Rtc[1]/16%16;
			Seg_Buf[4]=Rtc[1]%16;
			Seg_Buf[5]=13;
			Seg_Buf[6]=Rtc[2]/16%16;
			Seg_Buf[7]=Rtc[2]%16;
		break;
		case 1:
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=10;
			if(Input_Success_Flag==1)
			{
				Seg_Buf[4]=Input_Data/1000%10;
				Seg_Buf[5]=Input_Data/100%10;
				Seg_Buf[6]=Input_Data/10%10;
				Seg_Buf[7]=Input_Data/1%10;
				for(i=4;i<7;i++)
				{
					if(Seg_Buf[i]!=0)break;
					Seg_Buf[i]=10;
				}
			}
			else
				Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
		break;
		case 2:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=10;
			Seg_Buf[3]=Rtc_Input_Start[0]/16%16;
			Seg_Buf[4]=Rtc_Input_Start[0]%16;
			Seg_Buf[5]=13;
			Seg_Buf[6]=Rtc_Input_Start[1]/16%16;
			Seg_Buf[7]=Rtc_Input_Start[1]%16;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Realy_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Seg_Show_Mode==2?1:0;
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
	Relay(Realy_Buf);
	Beeper(Beeper_Buf);
}
/*初始化区域*/
void Init_Proc()
{
	
	RTC_Set(Rtc);
	EEPROM_Read(EEPROM_Save_Data,0,4);
	Input_Data_Old=EEPROM_Save_Data[2]<<8|EEPROM_Save_Data[3];
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
