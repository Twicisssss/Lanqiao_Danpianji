/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Key.H>
#include <Seg.H>
#include <onewire.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
float Temperature=0;
/*按键处理区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	
}
/*数码管处理区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Read_Temperature();
	
	Seg_Buf[0]=(unsigned char)Temperature/10%10;
	Seg_Buf[1]=(unsigned char)Temperature%10;
	Seg_Point[1]=1;
	Seg_Buf[2]=(unsigned char)(Temperature*10)%10;
	Seg_Buf[6]=Key_Down;
}
/*Led处理区域*/
void Led_Proc()
{
	Led_Buf[0]=1;
	Led_Buf[1]=1;
	Led_Buf[2]=1;
	Led_Buf[3]=1;
}
/*定时器区域*/
void Timer0_Isr(void) interrupt 1
{
	if(++Key_Slow==10)Key_Slow=0;
	if(++Seg_Slow==500)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
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
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
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

