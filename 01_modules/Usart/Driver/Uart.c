#include <Uart.H>
void Uart1_Init(void)	//9600bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xE6;			//设置定时初始值
	T2H = 0xFF;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	ES = 1;				//使能串口1中断
	EA=1;
}
void Uart_Send_Byte(unsigned char dat)
{
	SBUF=dat;
	while(TI==0);
	TI=0;
}
void Uart_Send_String(unsigned char *dat)
{
	while(*dat!='\0')
		Uart_Send_Byte(*dat++);
}
/*串口中断区域（放至main.c文件中）
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		Uart_Recv[Uart_Recv_Index]=SBUF;
		Uart_Recv_Index++;
		RI = 0;			//清除串口1接收中断请求位
	}
}
*/