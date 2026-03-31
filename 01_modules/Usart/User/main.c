/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Key.H>
#include <Led.H>
#include <Seg.H>
#include <Uart.H>
#include <stdio.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Uart_Slow;
unsigned char Uart_Recv_Index=0;
unsigned char Uart_Recv[10];
unsigned char Uart_Send[20];
unsigned char PC_Data_Num;
unsigned char User;
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
			sprintf(Uart_Send,"Hi！用户%03d\r\n",(unsigned int)User);
			Uart_Send_String(Uart_Send);
			User++;
		break;
		case 5:
			
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Seg_Buf[0]=PC_Data_Num/10;
	Seg_Buf[1]=PC_Data_Num%10;
	
	Seg_Point[4]=1;
}
/*Led显示区域*/
void Led_Proc()
{

}
/*串口区域*/
void Uart_Proc()
{
	if(Uart_Slow)return;
	Uart_Slow=1;
	
	if(Uart_Recv_Index==1)
	{
		PC_Data_Num=Uart_Recv[0]-48;//"-48":ASCII码//PC写多少MCU读多少
		Uart_Recv_Index=0;
	}
	if(Uart_Recv_Index==6)
	{
		if(Uart_Recv[0]=='L' && Uart_Recv[1]=='e' && Uart_Recv[2]=='d' && Uart_Recv[4]=='=')
		{
			Led_Buf[Uart_Recv[3]-48]=Uart_Recv[5]-48;
		}
		Uart_Recv_Index=0;
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
	if(++Uart_Slow==200)Uart_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
}
/*串口1中断区域*/
void Uart1_Isr(void) interrupt 4
{
	if (RI==1)				//检测串口1接收中断
	{
		Uart_Recv[Uart_Recv_Index]=SBUF;
		Uart_Recv_Index++;
		RI = 0;			//清除串口1接收中断请求位
	}
}

/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer0_Init();
	Uart1_Init();
	User=1;
	Uart_Send_String("Hello\r\n");
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
		Uart_Proc();
	}
}
