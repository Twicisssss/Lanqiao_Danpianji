#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Led.H>
#include <Uart.H>
#include <string.H>
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Uart_Buf[10]={0};
unsigned char Uart_Index;
unsigned char Uart_Rx_Flag;
unsigned char Uart_Tick;


//void Seg_Proc()
//{
//	if(Seg_Slow)return;
//	Seg_Slow=1;
//	
//}
//void Led_Proc()
//{
//	
//}
void Uart_Proc()
{
	unsigned char i;
	if(Uart_Index==0)return;
	if(Uart_Tick>=10)
	{
		Uart_Tick=0;
		Uart_Rx_Flag=0;

		if (Uart_Index == 8)
		{
			// 将接收到的8位数据存入Led_Buf数组
			for (i = 0; i < 8; i++)
			{
				Seg_Buf[i] = Uart_Buf[i] - '0'; // 将字符转换为数字
				Led_Buf[i] = (Seg_Buf[i]) ? 0 : 1;
			}
		}
		else
		{
			printf("error");
		}
		
		memset(Uart_Buf,0,Uart_Index);
		Uart_Index=0;
	}
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		Uart_Tick=0;
		Uart_Rx_Flag=1;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	
	if(Uart_Rx_Flag==1)Uart_Tick++;
}
void main()
{
	Uart1_Init();
	Sys_Init();
	Timer1_Init();
	while(1)
	{
//		Seg_Proc();
//		Led_Proc();
		Uart_Proc();
	}
}


