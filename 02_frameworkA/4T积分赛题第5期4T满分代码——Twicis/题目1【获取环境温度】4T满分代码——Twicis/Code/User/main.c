/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Key.H>
#include <Init.H>
#include <Seg.H>
#include <onewire.H>
#include <Uart.H>
/*变量声明区域*/
idata unsigned char Seg_Pos,Seg_Slow;
pdata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Uart_Buf[10]={0};
idata unsigned char Uart_Index;
idata unsigned char Uart_Rx_Flag;
idata unsigned char Uart_Tick;

idata float Temperature;
/*按键控制区域*/
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
		printf("Temp:%.2fC\r\n",Temperature);
	}
	
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read();
	
	Seg_Buf[7]=11;
	Seg_Buf[3]=(unsigned char)(Temperature)/10%10;
	Seg_Buf[4]=(unsigned char)(Temperature)/1%10+',';
	Seg_Buf[5]=(unsigned int)(Temperature*10)%10;
	Seg_Buf[6]=(unsigned int)(Temperature*100)%10;
}
/*串口控制区域*/
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
/*串口中断*/
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
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	
	if(Uart_Rx_Flag==1)Uart_Tick++;
}
/*初始化区域*/
void Init_Proc()
{
	while(Temperature_Read()==85);
	Sys_Init();
	Timer1_Init();
	Uart1_Init();
}
/*主函数区域*/
void main()
{
	Init_Proc();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Uart_Proc();
	}
}

