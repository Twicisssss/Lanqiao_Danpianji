#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <Uart.H>
#include <String.H>
#include <Motor.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Uart_Buf[10]={0};
unsigned char Uart_Index;
unsigned char Uart_Rx_Flag;
unsigned char Uart_Tick;

unsigned int Frequency=1000;
unsigned char DutyCycle=10;
unsigned int High_Time;
unsigned int Low_Time;
unsigned int PWM_Time_Count;
bit Signal_Level=0;

void PWM_Update()
{
	unsigned long Period_Xus;
	Period_Xus=1000000/Frequency;
	High_Time=(Period_Xus*(DutyCycle/100))/10;
	Low_Time=(Period_Xus-High_Time*10)/10;
	
	TR1=0;
	Motor(0);
	Signal_Level=0;
	PWM_Time_Count=0;
	TR1=1;
}
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Key_Down==10)
	{
		Frequency+=1000;
		if(Frequency==11000)Frequency=1000;
		PWM_Update();
	}
	else if(Key_Down==11)
	{
		DutyCycle+=10;
		if(DutyCycle==100)DutyCycle=10;
		printf("%bu%%",DutyCycle);
		PWM_Update();
	}
}
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
		
	Seg_Buf[0]=11;
	Seg_Buf[1]=Seg_Buf[2]=10;
	Seg_Buf[3]=Frequency==10000?Frequency/10000%10:10;
	Seg_Buf[4]=Frequency/1000%10;
	Seg_Buf[5]=Frequency/100%10;
	Seg_Buf[6]=Frequency/10%10;
	Seg_Buf[7]=Frequency/1%10;
}
void Uart_Proc()
{
	if(Uart_Index==0)return;
	if(Uart_Tick>=10)
	{
		
		
		Uart_Tick=0;
		memset(Uart_Buf,0,Uart_Index);
		Uart_Index=0;
	}
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		Uart_Rx_Flag=1;
		Uart_Tick=0;
		Uart_Buf[Uart_Index++]=SBUF;
		
		RI = 0;			//清除串口1接收中断请求位
		if(Uart_Index>10)
		{
			Uart_Index=0;
			memset(Uart_Buf,0,10);
		}
	}
}
void Timer1_Init(void)		//10微秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0xF6;				//设置定时初始值
	TH1 = 0xFF;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA=1;
}
void Timer1_Isr(void) interrupt 3
{
	PWM_Time_Count++;
	if(Signal_Level==0)
	{
		if(PWM_Time_Count>=Low_Time)
		{
			Motor(1);
			Signal_Level=1;
			PWM_Time_Count=0;
		}
	}
	else
	{
		if(PWM_Time_Count>=High_Time)
		{
			Motor(0);
			Signal_Level=0;
			PWM_Time_Count=0;
		}
	}
}
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
	if(++Seg_Slow==50)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	
	if(Uart_Rx_Flag)Uart_Tick++;
}

void main()
{
	Uart1_Init();
	Sys_Init();
	PWM_Update();
	Timer1_Init();
	Timer0_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Uart_Proc();
	}
}


