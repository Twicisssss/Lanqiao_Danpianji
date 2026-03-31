/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <Ultrasound.H>
#include <Uart.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

pdata unsigned char Uart_Buf[10] = {0}; // 串口接收缓冲区
idata unsigned char Uart_Rx_Index;      // 串口接收索引
idata unsigned char Uart_Recv_Tick;     // 串口接收时间标志
idata unsigned char Uart_Rx_Flag;
idata unsigned char Uart_MCU_String[20];

idata unsigned char Seg_Show_Mode=0;//【0-距离】【1-参数】
idata unsigned char US_Distance;
idata unsigned char Distance_Parameter=30;

idata unsigned char Time_200ms;
idata bit Time_200ms_Flag;

idata unsigned char i;
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
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
		break;
		case 8:
			if(Seg_Show_Mode==0)
				Distance_Parameter=US_Distance;
		break;
		case 12:
			if(Seg_Show_Mode==1)
			{
				if(Distance_Parameter>=246 && Distance_Parameter<=255)
					Distance_Parameter=Distance_Parameter;
				else
					Distance_Parameter+=10;
			}
		break;
		case 16:
			if(Seg_Show_Mode==1)
			{
				if(Distance_Parameter>=0 && Distance_Parameter<=9)
					Distance_Parameter=0;
				else
					Distance_Parameter-=10;
			}
		break;
		case 9://"串口发送"按键，按下将当前检测的距离数据发送给PC端的串口调试工具。
			printf("Distance:%dcm\r\n",(int)US_Distance);
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	US_Distance=US_Distance_Get();
	
	
	Seg_Buf[0]=11;Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[1]=1;
			Seg_Buf[5]=US_Distance/100%10;
			Seg_Buf[6]=US_Distance/10%10;
			Seg_Buf[7]=US_Distance/1%10;
			for(i=5;i<7;i++)
			{
				if(Seg_Buf[i]!=0)break;
				Seg_Buf[i]=10;
			}
		break;
		case 1:
			Seg_Buf[1]=2;
			Seg_Buf[5]=Distance_Parameter/100%10;
			Seg_Buf[6]=Distance_Parameter/10%10;
			Seg_Buf[7]=Distance_Parameter/1%10;
			for(i=5;i<7;i++)
			{
				if(Seg_Buf[i]!=0)break;
				Seg_Buf[i]=10;
			}
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	
	
	Led_Buf[2]=US_Distance>Distance_Parameter?Time_200ms_Flag:0;
}
/*串口控制区域*/
void Uart_Proc()
{
	if (Uart_Rx_Index == 0)
		return; // 无数据，直接返回
	if (Uart_Recv_Tick >= 10)
	{ // 若接收超时，对数据读取，清空缓存区
		Uart_Recv_Tick = 0;
		Uart_Rx_Flag = 0 ;
		//处理函数
		memset(Uart_Buf, 0, Uart_Rx_Index); // 清空接收数据
		Uart_Rx_Index = 0;
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
	if(Uart_Rx_Flag)Uart_Recv_Tick++;// 串口接收计时增加
	
	if(US_Distance>Distance_Parameter)
	{
		if(++Time_200ms==200)
		{
			Time_200ms=0;
			Time_200ms_Flag^=1;
		}
	}
	
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
}
/*串口中断区域*/
void Uart1_Isr(void) interrupt 4
{
	// 若接收到数据
	if (RI)
	{
		Uart_Rx_Flag = 1;                 // 接收标志
		Uart_Recv_Tick = 0;               // 清零接收时间标志
		Uart_Buf[Uart_Rx_Index++] = SBUF; // 将数据保存到缓冲区
		RI = 0;                           // 清除接收中断标志
		if (Uart_Rx_Index > 10)
		{
			Uart_Rx_Index = 0; // 防止溢出
			memset(Uart_Buf, 0, 10);
		}
	}
}
 /*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Uart1_Init();
	Timer1_Init();
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
