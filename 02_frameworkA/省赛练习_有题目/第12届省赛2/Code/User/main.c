/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Seg_Show_Mode=0;//【0-频率显示】、【1-周期显示】、【2-电压显示】
unsigned char V_Channel_Mode=1;//【1-光敏电阻】、【3-电压采集】

unsigned int Time_1000ms=0;
unsigned int Frequency;//NE555频率
unsigned int Frequency_Save_Data;
unsigned int Cycle_Xus;//周期

unsigned char AIN1_AD_Read_Data;//光敏电阻
unsigned char AIN3_AD_Read_Data;//电压采集
float AIN1_Light;
float AIN3_Voltage;
float Voltage_Save_Data;

unsigned char Key7_Keep_Flag=0;
bit Led_ON_OFF_Mode=1;
unsigned int Key7_Keep_1s=0;

/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
//长按S7按键超过1秒后松开按键，判定为S7长按键功能，禁用LED指示灯功能，所有LED处于熄灭状态；再次长按S7超过1秒后松开按键，恢复LED指示灯功能。
	if(Key_Old==7)
	{
		if(Key7_Keep_Flag!=2)
			{Key7_Keep_Flag=1;}
		else
			{Key7_Keep_Flag=2;}
	}
	else
	{
		Key7_Keep_Flag=0;
		Key7_Keep_1s=0;
	}
	
	switch(Key_Down)
	{
		case 4://S4：“界面切换”按键，按下切换【0-频率显示】、【1-周期显示】、【2-电压显示】界面。备注：每次从周期界面进入电压界面后，均为通道1电压显示界面。
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==2)V_Channel_Mode=1;
		break;
		case 5://S5：“电压通道切换”按键，在电压界面下，按下切换显示通道1和通道3电压测量结果。
			if(V_Channel_Mode==1)V_Channel_Mode=3;
			else if(V_Channel_Mode==3)V_Channel_Mode=1;
		break;
		case 6://S6：“通道3电压缓存”按键，按下保存当前采集到的通道3电压数据。在任何界面下均有效。
			Voltage_Save_Data=AIN3_Voltage;
		break;
		case 7://S7：“频率缓存”按键，按下保存当前采集到的频率数据。在任何界面下均有效。
			Frequency_Save_Data=Frequency;
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	AIN1_AD_Read_Data=AD_Read(0x03);
	AIN3_AD_Read_Data=AD_Read(0x01);
	AIN1_Light=AIN1_AD_Read_Data/51.0;
	AIN3_Voltage=AIN3_AD_Read_Data/51.0;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[5]=0;
			Seg_Buf[0]=11;//F
			if(Frequency>=0 && Frequency<10)
			{
				for(i=1;i<=6;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=10 && Frequency<100)
			{
				for(i=1;i<=5;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=100 && Frequency<1000)
			{
				for(i=1;i<=4;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=1000 && Frequency<10000)
			{
				for(i=1;i<=3;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=10000 && Frequency<100000)
			{
				for(i=1;i<=2;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[3]=Frequency/10000%10;
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=100000 && Frequency<1000000)
			{
				Seg_Buf[1]=10;
				Seg_Buf[2]=Frequency/100000%10;
				Seg_Buf[3]=Frequency/10000%10;
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else if(Frequency>=1000000 && Frequency<10000000)
			{
				Seg_Buf[1]=Frequency/1000000%10;
				Seg_Buf[2]=Frequency/100000%10;
				Seg_Buf[3]=Frequency/10000%10;
				Seg_Buf[4]=Frequency/1000%10;
				Seg_Buf[5]=Frequency/100%10;
				Seg_Buf[6]=Frequency/10%10;
				Seg_Buf[7]=Frequency/1%10;
			}
			else
			{
				for(i=1;i<=7;i++)
					{Seg_Buf[i]=10;}
			}
		break;
		case 1:
			Seg_Buf[0]=12;//n
			if(Cycle_Xus>=0 && Cycle_Xus<10)
			{
				for(i=1;i<=6;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=10 && Cycle_Xus<100)
			{
				for(i=1;i<=5;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=100 && Cycle_Xus<1000)
			{
				for(i=1;i<=4;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[5]=Cycle_Xus/100%10;
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=1000 && Cycle_Xus<10000)
			{
				for(i=1;i<=3;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[4]=Cycle_Xus/1000%10;
				Seg_Buf[5]=Cycle_Xus/100%10;
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=10000 && Cycle_Xus<100000)
			{
				for(i=1;i<=2;i++)
					{Seg_Buf[i]=10;}
				Seg_Buf[3]=Cycle_Xus/10000%10;
				Seg_Buf[4]=Cycle_Xus/1000%10;
				Seg_Buf[5]=Cycle_Xus/100%10;
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=100000 && Cycle_Xus<1000000)
			{
				Seg_Buf[1]=10;
				Seg_Buf[2]=Cycle_Xus/100000%10;
				Seg_Buf[3]=Cycle_Xus/10000%10;
				Seg_Buf[4]=Cycle_Xus/1000%10;
				Seg_Buf[5]=Cycle_Xus/100%10;
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else if(Cycle_Xus>=1000000 && Cycle_Xus<10000000)
			{
				Seg_Buf[1]=Cycle_Xus/1000000%10;
				Seg_Buf[2]=Cycle_Xus/100000%10;
				Seg_Buf[3]=Cycle_Xus/10000%10;
				Seg_Buf[4]=Cycle_Xus/1000%10;
				Seg_Buf[5]=Cycle_Xus/100%10;
				Seg_Buf[6]=Cycle_Xus/10%10;
				Seg_Buf[7]=Cycle_Xus/1%10;
			}
			else
			{
				for(i=1;i<=7;i++)
					{Seg_Buf[i]=10;}
			}
		break;
		case 2:
			Seg_Buf[0]=13;//U
			Seg_Buf[1]=14;//-
			Seg_Buf[2]=V_Channel_Mode;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			if(V_Channel_Mode==1)//【1-光敏电阻】
			{
			Seg_Buf[5]=(unsigned int)(AIN1_Light)%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(AIN1_Light*10)%10;
			Seg_Buf[7]=(unsigned int)(AIN1_Light*100)%10;
			}
			else if(V_Channel_Mode==3)//【3-电压采集】
			{
			Seg_Buf[5]=(unsigned int)AIN3_Voltage;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(AIN3_Voltage*10)%10;
			Seg_Buf[7]=(unsigned int)(AIN3_Voltage*100)%10;
			}
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
/*
1) L1:通道3的实时电压数据大于缓存电压数据，指示灯L1点亮，否则熄灭。
2) L2:实时频率值大于缓存频率数据，指示灯L2点亮，否则熄灭。
3) L3:处于频率界面，指示灯L3点亮，否则熄灭。
4) L4:处于周期界面，指示灯L4点亮，否则熄灭。
5) L5:处于电压界面，指示灯L5点亮，否则熄灭。
6) L6、L7、L8，处于熄灭状态。
*/	
	unsigned char i;
	if(Led_ON_OFF_Mode==1)
	{
		Led_Buf[0]=(AIN3_Voltage>Voltage_Save_Data)?1:0;
		Led_Buf[1]=(Frequency>Frequency_Save_Data)?1:0;
		Led_Buf[2]=(Seg_Show_Mode==0)?1:0;
		Led_Buf[3]=(Seg_Show_Mode==1)?1:0;
		Led_Buf[4]=(Seg_Show_Mode==2)?1:0;
		Led_Buf[5]=0;
		Led_Buf[6]=0;
		Led_Buf[7]=0;		
	}
	else if(Led_ON_OFF_Mode==0)
	{
		for(i=0;i<=7;i++)
			{Led_Buf[i]=0;}
	}
}
/*定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |=0x05;
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
	if(++Seg_Slow==200)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	if(++Time_1000ms==1000)
	{
		Time_1000ms=0;
		Frequency=(TH0<<8)|TL0;
		TH0=0;
		TL0=0;
		Cycle_Xus=(1*1000*1000)/Frequency;
	}
	
	if(Key7_Keep_Flag==1)
	{
		if(++Key7_Keep_1s==1000)
		{
			Key7_Keep_1s=0;
			Led_ON_OFF_Mode^=1;
			Key7_Keep_Flag=2;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
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

