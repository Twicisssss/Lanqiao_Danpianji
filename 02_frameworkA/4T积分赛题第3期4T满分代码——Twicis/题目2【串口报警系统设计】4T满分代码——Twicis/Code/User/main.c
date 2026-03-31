#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Key.H>
#include <Uart.H>
#include <string.H>
unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Uart_Buf[10]={0};
unsigned char Uart_Index;
unsigned char Uart_Rx_Flag;
unsigned char Uart_Tick;

unsigned char Seg_Show_Mode=0;//【0-发送界面】【1-报警界面】

unsigned char Data_Single[7]={10};
unsigned char Data_Single_Index=0;
unsigned char Data_Send_char=0;

void Key_Proc()
{
	unsigned char j;
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	Data_Send_char=0x00;
	switch(Key_Down)
	{
		case 4:
			if(Seg_Show_Mode==0 && Data_Single_Index>0)
			{
				for(j=0;j<Data_Single_Index;j++)
				{
					Data_Send_char|=Data_Single[j]<<(Data_Single_Index-1-j);
				}
				if(((Data_Send_char>=0x00) && (Data_Send_char<=0x1f))||(Data_Send_char==0x7f))
				{
					Seg_Show_Mode=1;
				}
				else
				{
					printf("%c",Data_Send_char);
					memset(Data_Single,10,Data_Single_Index);
					Data_Single_Index=0;
				}
			}
			else if(Seg_Show_Mode==1)
			{
				Seg_Show_Mode=0;
				memset(Data_Single,0,Data_Single_Index);
				Data_Single_Index=0;
			}
		break;
		case 8:
			if(Seg_Show_Mode==0 && Data_Single_Index<7)
			{
				Data_Single[Data_Single_Index++]=1;
			}
		break;
		case 12:
			if(Seg_Show_Mode==0 && Data_Single_Index<7 && Data_Single_Index>=1)
			{
				Data_Single[Data_Single_Index++]=0;
			}
		break;
	}
}
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
		
	if(Seg_Show_Mode==0)
	{
		Seg_Buf[0]=11;
		for(i=0;i<Data_Single_Index;i++)
		{
			Seg_Buf[7-i]=Data_Single[Data_Single_Index-1-i];
		}
		for(;i<7;i++)
		{
			Seg_Buf[7-i]=10;
		}
	}
	else
	{
		Seg_Buf[0]=Seg_Buf[1]=12;
		Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
	}
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
	if(++Seg_Slow==50)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	
	if(Uart_Rx_Flag==1)
		Uart_Tick++;
}
void main()
{
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


