#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Key.H>
unsigned char Key_Slow,Key_Down,Key_Up,Key_Val,Key_Old;
unsigned char Led_Pos,Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Led_Mode[8]={0,0,0,0,0,0,0,0};//【0-常亮】【1-闪烁】

unsigned char Key_Led_Num;

bit Press_Keep_Flag;
unsigned int Press_Time_1s;

unsigned int Time_100ms;
bit Time_Flash_Buf;
unsigned char Led_Reversal_Buf[8]={0,0,0,0,0,0,0,0};


void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Key_Down)
	{
		Press_Keep_Flag=1;
		Key_Led_Num=Key_Down-4;
	}
	if(Press_Time_1s==1000)
	{
		Led_Mode[Key_Led_Num]=1;
		Led_Reversal_Buf[Key_Led_Num]=Time_Flash_Buf;
	}
	if(Key_Up)
	{
		if(Press_Time_1s<1000)
		{
			Key_Led_Num=Key_Up-4;
			Led_Mode[Key_Led_Num]=0;
			Led_Reversal_Buf[Key_Led_Num]^=1;
		}
		
		Press_Time_1s=0;
		Press_Keep_Flag=0;
	}
	
		
}
void Led_Proc()
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		Led_Buf[i]=Led_Mode[i]==0?Led_Reversal_Buf[i]:Time_Flash_Buf;
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
	if(++Led_Pos==8)Led_Pos=0;
	Led_Disp(Led_Pos,Led_Buf[Led_Pos]);
	
	if(Press_Keep_Flag==1)
	{
		if(++Press_Time_1s>=1000)
			Press_Time_1s=1000;
	}
	
	if(++Time_100ms==100)
	{
		Time_100ms=0;
		Time_Flash_Buf^=1;
	}
	
}
void main()
{
	Sys_Init();
	Timer1_Init();
	while(1)
	{
		Key_Proc();
		Led_Proc();
	}
}



