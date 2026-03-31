#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <Uart.H>
#include <string.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Uart_Buf[10]={0};
unsigned char Uart_Index;
unsigned char Uart_Rx_Flag,Uart_Tick;

unsigned char Rtc[3]={23,59,57,};

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
		printf("Hour: %bu",Rtc[0]);
	}
	else if(Key_Down==8)
	{
		printf("Minute: %bu",Rtc[1]);
	}
	else if(Key_Down==12)
	{
		printf("Second: %bu",Rtc[2]);
	}
}
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	RTC_Read(Rtc);
	
	Seg_Buf[0]=Rtc[0]/10%10;
	Seg_Buf[1]=Rtc[0]%10;
	Seg_Buf[2]=11;
	Seg_Buf[3]=Rtc[1]/10%10;
	Seg_Buf[4]=Rtc[1]%10;
	Seg_Buf[5]=11;
	Seg_Buf[6]=Rtc[2]/10%10;
	Seg_Buf[7]=Rtc[2]%10;
}
void Uart_Proc()
{
	if(Uart_Index==0)return;
	if(Uart_Tick>=10)
	{
		Uart_Rx_Flag=0;
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
	
	if(Uart_Rx_Flag)Uart_Tick++;
}
void main()
{
	RTC_Set(Rtc);
	Sys_Init();
	Timer1_Init();
	Uart1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Uart_Proc();
	}
}


