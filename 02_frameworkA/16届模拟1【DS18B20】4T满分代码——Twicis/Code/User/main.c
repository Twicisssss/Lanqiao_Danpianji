/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
/*变量声明区域*/
idata unsigned char i;
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

idata unsigned char Seg_Show_Mode=0;//【0-温度】【1-校准值】【2-参数】
idata unsigned char Trigger_Mode=0;//【0-上触发】【1-下触发】

idata float Temperature;
idata float Temperature_Calibration=0;
idata float Temperature_Parameter=26;
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
		break;
		case 5:
			if(++Trigger_Mode==2)Trigger_Mode=0;
		break;
		case 8:
			if(Seg_Show_Mode==1)
			{
				if(--Temperature_Calibration==-100)
					Temperature_Calibration=-99;
			}
			else if(Seg_Show_Mode==2)
			{
				if(--Temperature_Parameter==-100)
					Temperature_Parameter=-99;
			}
		break;
		case 9:
			if(Seg_Show_Mode==1)
			{
				if(++Temperature_Calibration==100)
					Temperature_Calibration=99;
			}
			else if(Seg_Show_Mode==2)
			{
				if(++Temperature_Parameter==100)
					Temperature_Parameter=99;
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read()+Temperature_Calibration;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[6]=(unsigned char)(Temperature)/1%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
		break;
		case 1:
			Seg_Point[6]=0;
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(Temperature_Calibration>=0)
			{
				Seg_Buf[5]=(unsigned char)(Temperature_Calibration)/100%10;
				Seg_Buf[6]=(unsigned char)(Temperature_Calibration)/10%10;
				Seg_Buf[7]=(unsigned char)(Temperature_Calibration)/1%10;
				for(i=5;i<7;i++)
				{
					if(Seg_Buf[i]!=0)break;
					Seg_Buf[i]=10;
				}
			}
			else if(Temperature_Calibration>-10 && Temperature_Calibration<0)
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=14;
				Seg_Buf[7]=(unsigned char)(-Temperature_Calibration)/1%10;
			}
			else if(Temperature_Calibration>-100 && Temperature_Calibration<=-10)
			{
				Seg_Buf[5]=14;
				Seg_Buf[6]=(unsigned char)(-Temperature_Calibration)/10%10;
				Seg_Buf[7]=(unsigned char)(-Temperature_Calibration)/1%10;
			}
		break;
		case 2:
			Seg_Buf[0]=13;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(Temperature_Parameter>=0)
			{
				Seg_Buf[5]=(unsigned char)(Temperature_Parameter)/100%10;
				Seg_Buf[6]=(unsigned char)(Temperature_Parameter)/10%10;
				Seg_Buf[7]=(unsigned char)(Temperature_Parameter)/1%10;
				for(i=5;i<7;i++)
				{
					if(Seg_Buf[i]!=0)break;
					Seg_Buf[i]=10;
				}
			}
			else if(Temperature_Parameter>-10 && Temperature_Parameter<0)
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=14;
				Seg_Buf[7]=(unsigned char)(-Temperature_Parameter)/1%10;
			}
			else if(Temperature_Parameter>-100 && Temperature_Parameter<=-10)
			{
				Seg_Buf[5]=14;
				Seg_Buf[6]=(unsigned char)(-Temperature_Parameter)/10%10;
				Seg_Buf[7]=(unsigned char)(-Temperature_Parameter)/1%10;
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
	Led_Buf[2]=Seg_Show_Mode==2?1:0;
	
	Led_Buf[3]=Trigger_Mode==0?1:0;
	Led_Buf[4]=Trigger_Mode==1?1:0;
	
	if(Trigger_Mode==0)
		Led_Buf[7]=Temperature>Temperature_Parameter?1:0;
	else
		Led_Buf[7]=Temperature<Temperature_Parameter?1:0;
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
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
}
/*初始化区域*/
void Init_Proc()
{
	while(Temperature_Read()==85);
	Sys_Init();
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
	}
}
