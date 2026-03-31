/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Key.H>
#include <Seg.H>
#include <iic.H>

/*变量声明区域*/
idata unsigned char Key_Slow;
idata unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow;
idata unsigned char Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0;
idata unsigned char Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-频率】【1-周期】【2-电压】
idata unsigned char AD_Channel_Mode=1;//【1-光敏电阻】【3-电位器Rb2】
idata bit LED_ON_OFF_Mode=1;//【1-开启】【1-禁用】

idata float V_AIN1_Light;//【1-光敏电阻】
idata float V_AIN3_Rb2;//【3-电位器Rb2】
idata float V_AIN3_Save;


idata unsigned int Time_1000ms;
idata unsigned int Frequency;
idata unsigned int Frequency_Save;
idata unsigned int Cycle_Time;

idata bit Long_Press_Time_Flag;
idata unsigned int Time_1s;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Key_Down==7)
	{
		Long_Press_Time_Flag=1;
	}
	if(Key_Up==7)
	{
		if(Time_1s==1000)
		{
			LED_ON_OFF_Mode^=1;
		}
		else
		{
			Frequency_Save=Frequency;
		}
		Time_1s=0;
		Long_Press_Time_Flag=0;
	}
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==2)AD_Channel_Mode=1;
		break;
		case 5:
			if(AD_Channel_Mode==1)
				AD_Channel_Mode=3;
			else 
				AD_Channel_Mode=1;
		break;
		case 6:
			V_AIN3_Save=V_AIN3_Rb2;
		break;
//		case 7:
//			Frequency_Save=Frequency;
//		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i;
	
	switch(Seg_Slow)
	{
		case 30:
			V_AIN1_Light = AD_Read(1)/51.0;
		break;
		case 60:
			Cycle_Time = 1000000 / Frequency; // 计算周期 us
		break;
		case 90:
			V_AIN3_Rb2 = AD_Read(3)/51.0;
		break;
	}
  
	if(Seg_Slow)return;
	Seg_Slow=1;
		
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[5]=0;
			Seg_Buf[0]=11;
			Seg_Buf[1]=Frequency/1000000%10;
			Seg_Buf[2]=Frequency/100000%10;
			Seg_Buf[3]=Frequency/10000%10;
			Seg_Buf[4]=Frequency/1000%10;
			Seg_Buf[5]=Frequency/100%10;
			Seg_Buf[6]=Frequency/10%10;
			Seg_Buf[7]=Frequency/1%10;
			if(Seg_Buf[1]==0)
				Seg_Buf[1]=10;
			for(i=1;i<=7;i++)
			{
				if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
					Seg_Buf[i]=10;
			}
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Cycle_Time/1000000%10;
			Seg_Buf[2]=Cycle_Time/100000%10;
			Seg_Buf[3]=Cycle_Time/10000%10;
			Seg_Buf[4]=Cycle_Time/1000%10;
			Seg_Buf[5]=Cycle_Time/100%10;
			Seg_Buf[6]=Cycle_Time/10%10;
			Seg_Buf[7]=Cycle_Time/1%10;
			if(Seg_Buf[1]==0)
				Seg_Buf[1]=10;
			for(i=1;i<=7;i++)
			{
				if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
					Seg_Buf[i]=10;
			}
		break;
		case 2:
			Seg_Buf[0]=13;
			Seg_Buf[1]=14;
			Seg_Buf[2]=AD_Channel_Mode;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Point[5]=1;
			if(AD_Channel_Mode==1)
			{
				Seg_Buf[5]=(unsigned char)(V_AIN1_Light)%10;
				Seg_Buf[6]=(unsigned char)(V_AIN1_Light*10)%10;
				Seg_Buf[7]=(unsigned int)(V_AIN1_Light*100)%10;
			}
			else
			{
				Seg_Buf[5]=(unsigned char)(V_AIN3_Rb2)%10;
				Seg_Buf[6]=(unsigned char)(V_AIN3_Rb2*10)%10;
				Seg_Buf[7]=(unsigned int)(V_AIN3_Rb2*100)%10;
			}
		break;
		
	}
}
/*Led显示区域*/
void Led_Proc()
{
	unsigned char j;
	Beeper_Buf=Relay_Buf=0;
	if(LED_ON_OFF_Mode==1)
	{
		Led_Buf[0]=(V_AIN3_Rb2>V_AIN3_Save)?1:0;
		Led_Buf[1]=(Frequency>Frequency_Save)?1:0;
		Led_Buf[2]=(Seg_Show_Mode==0)?1:0;
		Led_Buf[3]=(Seg_Show_Mode==1)?1:0;
		Led_Buf[4]=(Seg_Show_Mode==2)?1:0;
	}
	else
	{
		for(j=0;j<=7;j++)
			Led_Buf[j]=0;
	}
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	
	if(++Time_1000ms==1000)
	{
		TR0 = 0;
		Time_1000ms=0;
		Frequency=(TH0<<8)|TL0;
//		Cycle_Time=1000000/Frequency;
		TH0=TL0=0;
		TR0 = 1;
	}
	
	
	if(Long_Press_Time_Flag==1)
	{
		if(++Time_1s>=1000)
		{
			Time_1s=1000;
		}
	}
	else
	{
		Time_1s=0;
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
	
	AD_Read(1);
	
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
