/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i;

idata unsigned char Seg_Show_Mode=0;//【0-模式界面】【1-输出界面】
idata unsigned char Ctrl_Mode=0;//【0-模式1温度控制】【1-模式2光照度控制】

idata float Temperature;
idata unsigned char ADC_Light;
idata float DAC_Voltage;
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
			if(Seg_Show_Mode==0)//【0-模式界面】
			{
				if(++Ctrl_Mode==2)Ctrl_Mode=0;
			}
		break;
		case 5:
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read();
	ADC_Light=AD_Read(0x01);
	if(Ctrl_Mode==0)//【0-模式1温度控制】
	{
		if(Temperature>10 && Temperature<40)
			DAC_Voltage=(4*Temperature-10)/30.0;
		else if(Temperature<=10)
			DAC_Voltage=1.0;
		else if(Temperature>=40)
			DAC_Voltage=5.0;
	}
	else//【1-模式2光照度控制】
	{
		if(ADC_Light>10 && ADC_Light<240)
			DAC_Voltage=(4*ADC_Light+190)/230.0;
		else if(ADC_Light<=10)
			DAC_Voltage=1.0;
		else if(ADC_Light>=240)
			DAC_Voltage=5.0;
	}
	
	switch(Seg_Show_Mode)
	{
		case 0://【0-模式界面】
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
		if(Ctrl_Mode==0)//【0-模式1温度控制】
		{
			Seg_Buf[0]=1;
			Seg_Buf[5]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[6]=(unsigned char)(Temperature)/1%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
		}
		else//【1-模式2光照度控制】
		{
			Seg_Point[6]=0;
			Seg_Buf[0]=2;
			Seg_Buf[5]=ADC_Light/100%10;
			Seg_Buf[6]=ADC_Light/10%10;
			Seg_Buf[7]=ADC_Light/1%10;
			for(i=5;i<7;i++)
			{
				if(Seg_Buf[i]!=0)break;
				Seg_Buf[i]=10;
			}
		}
		break;
		case 1://【1-输出界面】
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=(unsigned char)(DAC_Voltage)%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned char)(DAC_Voltage*10)%10;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Ctrl_Mode==0?1:0;
	Led_Buf[1]=Ctrl_Mode==1?1:0;
	DA_Write(DAC_Voltage*51.0);
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
