/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Key.H>
#include <Led.H>
#include <Seg.H>
#include <Init.H>
#include <ds1302.H>
#include <onewire.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char ucRtc[3]={0x23,0x59,0x50};
unsigned char Alarm[3]={0x00,0x00,0x00};
unsigned char ucRtc_Change[3]={0x23,0x59,0x50};
unsigned char Alarm_Change[3]={0x00,0x00,0x00};
bit Alarm_Flag=0;

unsigned char Seg_Show_Mode=0;//显示模式		0-时钟；1-时钟改时；2-闹钟改时
unsigned char TIME_TEMPE_Mode=0;//显示模式		0-时间；1-温度
unsigned char ucRtc_Index=0;//“时钟设置”选择修改目标（该目标进行闪烁）		0-时；1-分；2-秒

unsigned int Flash_Time_1s=0;
bit Flash_Flag=0;
unsigned int Flash_Time_200ms=0;
bit Flash_200ms_Flag=0;
unsigned int Led_Last_5s=0;

float Temperature=0;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;

	if(Seg_Show_Mode==0)
	{
		if(Key_Old==4)
			{TIME_TEMPE_Mode=1;}
		else
			{TIME_TEMPE_Mode=0;}

	}
	if(Alarm_Flag==1)
	{
		if(Key_Down!=0)
		{
			Alarm_Flag=0;
			Flash_Time_200ms=0;
			Flash_200ms_Flag=0;
			Led_Last_5s=0;
		}
		return;
	}
	
	switch(Key_Down)
	{
		case 4://减按键（只在“时钟”/“闹钟”状态下有效）【在 “时钟”状态下，按下显示温度数据，松开返回“时钟显示”界面】
			if(Seg_Show_Mode==1)
			{
				ucRtc_Change[ucRtc_Index-1]--;
				if(ucRtc_Change[ucRtc_Index-1]%16==0x0f)ucRtc_Change[ucRtc_Index-1]-=6;
				if(ucRtc_Change[ucRtc_Index-1]==0xf9)ucRtc_Change[ucRtc_Index-1]=0x00;
			}
			else if(Seg_Show_Mode==2)
			{
				Alarm_Change[ucRtc_Index-1]--;
				if(Alarm_Change[ucRtc_Index-1]%16==0x0f)Alarm_Change[ucRtc_Index-1]-=6;
				if(Alarm_Change[ucRtc_Index-1]==0xf9)Alarm_Change[ucRtc_Index-1]=0x00;
			}
		break;
		case 5://加按键（只在“时钟”/“闹钟”状态下有效）
			if(Seg_Show_Mode==1)
			{
				ucRtc_Change[ucRtc_Index-1]++;
				if(ucRtc_Change[ucRtc_Index-1]%16==0x0a)ucRtc_Change[ucRtc_Index-1]+=6;
				if(ucRtc_Index==1)
					{if(ucRtc_Change[1]==0x24)ucRtc_Change[1]=0x23;}
				if(ucRtc_Index==2||ucRtc_Index==3)
					{if(ucRtc_Change[ucRtc_Index-1]==0x60)ucRtc_Change[ucRtc_Index-1]=0x59;}
			}
			else if(Seg_Show_Mode==2)
			{
				Alarm_Change[ucRtc_Index-1]++;
				if(Alarm_Change[ucRtc_Index-1]%16==0x0a)Alarm_Change[ucRtc_Index-1]+=6;
				if(ucRtc_Index==1)
					{if(Alarm_Change[1]==0x24)Alarm_Change[1]=0x23;}
				if(ucRtc_Index==2||ucRtc_Index==3)
					{if(Alarm_Change[ucRtc_Index-1]==0x60)Alarm_Change[ucRtc_Index-1]=0x59;}
			}
		break;
		case 6://“闹钟设置”按键：按下进入闹钟时间设置功能，数码管显示当前设定的闹钟时间。
			if(Seg_Show_Mode==0||Seg_Show_Mode==2)
			{
				Seg_Show_Mode=2;
				if(++ucRtc_Index==4)
				{
					Seg_Show_Mode=0;
					ucRtc_Index=0;
					Alarm[0]=Alarm_Change[0];
					Alarm[1]=Alarm_Change[1];
					Alarm[2]=Alarm_Change[2];
				}
			}
		break;
		case 7://“时钟设置”按键，按下切换选择待选时、分、秒，选择目标以 1 秒为间隔亮灭，注意数据边界
			if(Seg_Show_Mode==0||Seg_Show_Mode==1)
			{
				Seg_Show_Mode=1;
				if(++ucRtc_Index==4)
				{
					Seg_Show_Mode=0;
					ucRtc_Index=0;
					ucRtc[0]=ucRtc_Change[0];
					ucRtc[1]=ucRtc_Change[1];
					ucRtc[2]=ucRtc_Change[2];
					Set_RTC(ucRtc);
				}
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Read_Temperature();
	Read_RTC(ucRtc);
	
	if(TIME_TEMPE_Mode==0)
	{
		switch(Seg_Show_Mode)
		{
			case 0:
				Seg_Buf[0]=ucRtc[0]/16;
				Seg_Buf[1]=ucRtc[0]%16;
				Seg_Buf[2]=11;
				Seg_Buf[3]=ucRtc[1]/16;
				Seg_Buf[4]=ucRtc[1]%16;
				Seg_Buf[5]=11;
				Seg_Buf[6]=ucRtc[2]/16;
				Seg_Buf[7]=ucRtc[2]%16;
				
				ucRtc_Change[0]=ucRtc[0];
				ucRtc_Change[1]=ucRtc[1];
				ucRtc_Change[2]=ucRtc[2];
			break;
			case 1:
				switch(ucRtc_Index)
				{
					case 1:
							Seg_Buf[0]=Flash_Flag?ucRtc_Change[0]/16:10;
							Seg_Buf[1]=Flash_Flag?ucRtc_Change[0]%16:10;
							Seg_Buf[3]=ucRtc_Change[1]/16;
							Seg_Buf[4]=ucRtc_Change[1]%16;
							Seg_Buf[6]=ucRtc_Change[2]/16;
							Seg_Buf[7]=ucRtc_Change[2]%16;
					break;
					case 2:
							Seg_Buf[0]=ucRtc_Change[0]/16;
							Seg_Buf[1]=ucRtc_Change[0]%16;
							Seg_Buf[3]=Flash_Flag?ucRtc_Change[1]/16:10;
							Seg_Buf[4]=Flash_Flag?ucRtc_Change[1]%16:10;
							Seg_Buf[6]=ucRtc_Change[2]/16;
							Seg_Buf[7]=ucRtc_Change[2]%16;
					break;
					case 3:
							Seg_Buf[0]=ucRtc_Change[0]/16;
							Seg_Buf[1]=ucRtc_Change[0]%16;
							Seg_Buf[3]=ucRtc_Change[1]/16;
							Seg_Buf[4]=ucRtc_Change[1]%16;
							Seg_Buf[6]=Flash_Flag?ucRtc_Change[2]/16:10;
							Seg_Buf[7]=Flash_Flag?ucRtc_Change[2]%16:10;
					break;					
				}
			break;
			case 2:
				switch(ucRtc_Index)
				{
					case 1:
							Seg_Buf[0]=Alarm_Change[0]/16;
							Seg_Buf[1]=Alarm_Change[0]%16;
							Seg_Buf[3]=Alarm_Change[1]/16;
							Seg_Buf[4]=Alarm_Change[1]%16;
							Seg_Buf[6]=Alarm_Change[2]/16;
							Seg_Buf[7]=Alarm_Change[2]%16;
					break;
					case 2:
							Seg_Buf[0]=Alarm_Change[0]/16;
							Seg_Buf[1]=Alarm_Change[0]%16;
							Seg_Buf[3]=Alarm_Change[1]/16;
							Seg_Buf[4]=Alarm_Change[1]%16;
							Seg_Buf[6]=Alarm_Change[2]/16;
							Seg_Buf[7]=Alarm_Change[2]%16;
					break;
					case 3:
							Seg_Buf[0]=Alarm_Change[0]/16;
							Seg_Buf[1]=Alarm_Change[0]%16;
							Seg_Buf[3]=Alarm_Change[1]/16;
							Seg_Buf[4]=Alarm_Change[1]%16;
							Seg_Buf[6]=Alarm_Change[2]/16;
							Seg_Buf[7]=Alarm_Change[2]%16;
					break;					
				}
			break;
		}
	}
	else
	{
			Seg_Buf[0]=10;
			Seg_Buf[1]=10;
			Seg_Buf[2]=10;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)Temperature/10;
			Seg_Buf[6]=(unsigned char)Temperature%10;
			Seg_Buf[7]=12;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	if(Alarm_Flag==1)
	{
		Led_Buf[0]=Flash_200ms_Flag?1:0;
	}
	else
	{
		Led_Buf[0]=0;
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
	if(++Seg_Slow==500)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	if(Seg_Show_Mode!=0)
	{
		if(++Flash_Time_1s==1000)
		{
			Flash_Time_1s=0;
			Flash_Flag=~Flash_Flag;
		}
	}
	if(ucRtc[0]==Alarm[0] && ucRtc[1]==Alarm[1] && ucRtc[2]==Alarm[2])
		{Alarm_Flag=1;}
	if(Alarm_Flag==1)
	{
		if(++Flash_Time_200ms==200)
		{
			Flash_Time_200ms=0;
			Led_Last_5s++;
			Flash_200ms_Flag=~Flash_200ms_Flag;
		}
		if(Led_Last_5s==25)
		{
			Alarm_Flag=0;
			Led_Last_5s=0;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer0_Init();
	Set_RTC(ucRtc);
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

