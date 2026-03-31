/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow;
idata unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow;
idata unsigned char Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata bit Beeper_Buf=0;
idata bit Realy_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-温度】【1-参数】【2-DAC】
idata unsigned char DAC_OutPut_Mode=0;//【0-温度参数】【1-实时温度】
idata float Temperature;
idata unsigned char Temperature_Parameter[2]={25,25};//【0-参数值】【1-修改值】
idata float Voltage;
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
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)
			{
				Temperature_Parameter[1]=Temperature_Parameter[0];
			}
			else if(Seg_Show_Mode==2)
			{
				Temperature_Parameter[0]=Temperature_Parameter[1];
			}
		break;
		case 5:
			if(++DAC_OutPut_Mode==2)DAC_OutPut_Mode=0;
		break;
		case 8:
			if(Seg_Show_Mode==1)
			{
				if(--Temperature_Parameter[1]==255)Temperature_Parameter[1]=0;
			}
		break;
		case 9:
			if(Seg_Show_Mode==1)
			{
				if(++Temperature_Parameter[1]==100)Temperature_Parameter[1]=99;
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	Temperature=Temperature_Read();
	if(DAC_OutPut_Mode==0)
	{
		if(Temperature<Temperature_Parameter[0])
		{
			Voltage=0.0;
		}
		else
		{
			Voltage=5.0;
		}
	}
	else
	{
		if(Temperature>20 && Temperature<40)
		{
			Voltage=(3*Temperature)/20-2;
		}
		else if(Temperature<=20)
		{
			Voltage=1.0;
		}
		else if(Temperature>=40)
		{
			Voltage=4.0;
		}
	}
	DA_Write(Voltage*51);
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=10;
			Seg_Buf[4]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[5]=(unsigned char)(Temperature)/1%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Temperature*10)%10;
			Seg_Buf[7]=(unsigned int)(Temperature*100)%10;
		break;
		case 1:
			Seg_Point[5]=0;
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=Temperature_Parameter[1]/10%10;
			Seg_Buf[7]=Temperature_Parameter[1]/1%10;
		break;
		case 2:
			Seg_Buf[0]=13;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned int)(Voltage)/1%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Voltage*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage*100)%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Beeper_Buf=Realy_Buf=0;
	Led_Buf[0]=DAC_OutPut_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==0?1:0;
	Led_Buf[2]=Seg_Show_Mode==1?1:0;
	Led_Buf[3]=Seg_Show_Mode==2?1:0;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Realy_Buf);
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
