/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Key.H>
#include <Led.H>
#include <Seg.H>
#include <UltraSound.H>

/*变量声明区域*/
unsigned char Key_Slow=0;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow=0;
unsigned char Seg_Pos=0;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char UltraSound_Length;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=0;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
		
		break;
		case 5:
			
		break;
	}
}
/*数码管显示*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=0;
	
	UltraSound_Length=US_Length_Read();
	
	Seg_Buf[0]=UltraSound_Length/100%10;
	Seg_Buf[1]=UltraSound_Length/10%10;
	Seg_Buf[2]=UltraSound_Length/1%10;
	
	Seg_Point[7]=1;
	
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=1;
}
/*定时器0初始化部分*/
void Timer0_Isr(void) interrupt 1
{
	if(++Key_Slow==20)Key_Slow=0;
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
/*全局初始化区域*/
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
