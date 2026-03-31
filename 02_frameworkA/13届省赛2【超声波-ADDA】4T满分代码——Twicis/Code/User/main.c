/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Key.H>
#include <Seg.H>
#include <Ultrasound.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Down,Key_Up,Key_Val,Key_Old;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata bit Beeper_Buf=0;
idata bit Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-电压】【1-测距】【2-参数】
idata unsigned char Parameter_Index=0;//【0-上限】【1-下限】

idata float Voltage_RB2_ADC;
idata float Rb2_DAC_Voltage;
idata float Voltage_Parameter[2]={4.5,0.5};//参数实际值【0-上限】【1-下限】
idata float Voltage_Parameter_Set[2]={4.5,0.5};//参数修改值【0-上限】【1-下限】

idata bit US_ON_OFF_Mode=0;//【0-关闭】【1-开启】
idata unsigned char US_Distance;

idata unsigned char Time_100ms;
idata bit Time_100ms_Flag;
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
			if(Seg_Show_Mode==2)
			{
				Parameter_Index=0;
				Voltage_Parameter_Set[0]=Voltage_Parameter[0];
				Voltage_Parameter_Set[1]=Voltage_Parameter[1];
			}
			else if(Seg_Show_Mode==0)
			{
				if(Voltage_Parameter_Set[0]>Voltage_Parameter_Set[1])
				{
					Voltage_Parameter[0]=Voltage_Parameter_Set[0];
					Voltage_Parameter[1]=Voltage_Parameter_Set[1];
				}
			}
		break;
		case 5:
			if(Seg_Show_Mode==2)
				Parameter_Index^=1;
		break;
		case 6:
			if(Seg_Show_Mode==2)
			{
				if(Parameter_Index==0)
				{
					Voltage_Parameter_Set[0]+=0.5;
					if(Voltage_Parameter_Set[0]==5.5)
						Voltage_Parameter_Set[0]=0.5;
				}
				else if(Parameter_Index==1)
				{
					Voltage_Parameter_Set[1]+=0.5;
					if(Voltage_Parameter_Set[1]==5.5)
						Voltage_Parameter_Set[1]=0.5;
				}
			}
		break;
		case 7:
			if(Seg_Show_Mode==2)
			{
				if(Parameter_Index==0)
				{
					Voltage_Parameter_Set[0]-=0.5;
					if(Voltage_Parameter_Set[0]==0.0)
						Voltage_Parameter_Set[0]=5.0;
				}
				else if(Parameter_Index==1)
				{
					Voltage_Parameter_Set[1]-=0.5;
					if(Voltage_Parameter_Set[1]==0.0)
						Voltage_Parameter_Set[1]=5.0;
				}
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	Voltage_RB2_ADC=AD_Read(3)/51.0;
	if(Voltage_RB2_ADC>Voltage_Parameter[1] && Voltage_RB2_ADC<Voltage_Parameter[0])
		US_ON_OFF_Mode=1;
	else
		US_ON_OFF_Mode=0;
	
	
	if(US_ON_OFF_Mode==1)
	{
		US_Distance=US_Distance_Get();
		if(US_Distance>20 && US_Distance<80)
			Rb2_DAC_Voltage=(2*US_Distance-10)/30.0;
		else if(US_Distance<=20)
			Rb2_DAC_Voltage=1.0;
		else if(US_Distance>=80)
			Rb2_DAC_Voltage=5.0;
	}
	else
		Rb2_DAC_Voltage=0;
	DA_Write(Rb2_DAC_Voltage*51);
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[3]=Seg_Point[6]=0;
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)(Voltage_RB2_ADC)%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Voltage_RB2_ADC*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_RB2_ADC*100)%10;
		break;
		case 1:
			Seg_Point[5]=0;
			Seg_Buf[0]=13;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(US_ON_OFF_Mode==1)
			{
				Seg_Buf[5]=US_Distance/100%10;
				Seg_Buf[6]=US_Distance/10%10;
				Seg_Buf[7]=US_Distance/1%10;
				for(i=5;i<=7;i++)
				{
					if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
						Seg_Buf[i]=10;
				}
			}
			else
			{
				Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=14;
			}
		break;
		case 2:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=10;
			Seg_Buf[3]=(unsigned char)(Voltage_Parameter_Set[0])%10;
			Seg_Point[3]=1;
			Seg_Buf[4]=(unsigned char)(Voltage_Parameter_Set[0]*10)%10;
			Seg_Buf[5]=10;
			Seg_Buf[6]=(unsigned char)(Voltage_Parameter_Set[1])%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned char)(Voltage_Parameter_Set[1]*10)%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=Seg_Show_Mode==0?1:0;//【0-电压】【1-测距】【2-参数】
	Led_Buf[1]=Seg_Show_Mode==1?1:0;//【0-电压】【1-测距】【2-参数】
	Led_Buf[2]=Seg_Show_Mode==2?1:0;//【0-电压】【1-测距】【2-参数】
	
	Led_Buf[7]=US_ON_OFF_Mode==1?Time_100ms_Flag:0;
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
	
	
	if(++Time_100ms==100)
	{
		Time_100ms=0;
		Time_100ms_Flag^=1;
	}
	
	
	
	
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
