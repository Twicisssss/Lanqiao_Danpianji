#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <Led.H>
#include <iic.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Seg_Show_Mode=0;//【0-欢迎】【1-工作】【2-设置】

unsigned char ADC_Light_Data;
bit Light_State=1;//【0-挡光】【1-未挡光】
bit Occlusion_First_Flag=1;//【0-不是第一次挡光】【1-是第一次挡光】
unsigned char Remaining_Time;
unsigned int Time_Reduce_1000ms;
unsigned char Time_Parameter=5;
bit Time_Over_Flag=0;//【0-计时未完成】【1-计时完成】
bit Occlusion_Keep_Back=0;

unsigned int Time_3s;
bit Time_3s_Flag;


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
		if(Seg_Show_Mode==0)Seg_Show_Mode=2;
		else if(Seg_Show_Mode==2)
		{
			Time_3s=0;
			Time_Parameter+=5;
			if(Time_Parameter==35)Time_Parameter=5;
		}
	}
}
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	ADC_Light_Data=AD_Read(0x01);
	
	if(Time_Over_Flag==0)
	{
		if(ADC_Light_Data<40)
		{
			if(Occlusion_Keep_Back==0)
			{
				Light_State=0;
				if(Occlusion_First_Flag==1)
				{
					Occlusion_First_Flag=0;
					Remaining_Time=Time_Parameter;
				}
				Seg_Show_Mode=1;
			}
		}
		else
		{
			Light_State=1;
			Occlusion_Keep_Back=0;
		}
	}
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;
			Seg_Buf[1]=12;
			Seg_Buf[2]=13;
			Seg_Buf[3]=13;
			Seg_Buf[4]=0;
			Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
		break;
		case 1:
			Seg_Buf[0]=14;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=Remaining_Time>=10?Remaining_Time/10:10;
			Seg_Buf[7]=Remaining_Time%10;
		break;
		case 2:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=Time_Parameter>=10?Time_Parameter/10:10;
			Seg_Buf[7]=Time_Parameter%10;
		break;
	}
}
void Led_Proc()
{
	Relay_Buf=(Light_State==0);

	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Seg_Show_Mode==2?1:0;
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
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Relay(Relay_Buf);
	
	
	if(Light_State==0)
	{
		if(++Time_Reduce_1000ms==1000)
		{
			Time_Reduce_1000ms=0;
			Remaining_Time--;
			if(Remaining_Time==0)
			{
				Time_Over_Flag=1;
				Light_State=1;
				Occlusion_Keep_Back=1;
			}
		}
	}
	
	if(Seg_Show_Mode==1)
	{
		if(Light_State==1)
		{
			if(++Time_3s==3000)
			{
				Time_3s=0;
				Seg_Show_Mode=0;
				Time_Over_Flag=0;
				Occlusion_First_Flag=1;
			}
		}
		else
		{
			Time_3s=0;
		}
	}
	if(Seg_Show_Mode==2)
	{
		if(++Time_3s==3000)
		{
			Time_3s=0;
			Seg_Show_Mode=0;
			Time_Over_Flag=0;
			Occlusion_First_Flag=1;
		}
	}
}
void main()
{
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}


