/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow;
idata unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow;
idata unsigned char Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beep_Buf=0;
idata unsigned char Relay_Buf=0;

idata unsigned char Seg_Show_Mode=0;//【0-数据】【1-参数】【2-计数】
idata float Voltage_AD_Data=0;
idata float Voltage_AD_Data_Old=0;
idata float Voltage_Parameter[2]={3.0,3.0};//【0-参数值】【1-参数修改值】
idata unsigned char Key_Errors=0;

idata bit Voltage_Below_Parameter_Flag=0;
idata unsigned int Time_5000ms;
idata bit Time_5000ms_Flag=0;

pdata unsigned int Count=0;
pdata unsigned char Parameter_EEPROM[1];
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
		case 12://S12："显示界面切换"按键，按下切换选择【0-数据】【1-参数】【2-计数】界面
			Key_Errors=0;
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)//离开【0-数据】界面，进入【1-参数】界面
				Voltage_Parameter[1]=Voltage_Parameter[0];
			if(Seg_Show_Mode==2)//离开【1-参数】界面，进入【2-计数】界面
			{
				Voltage_Parameter[0]=Voltage_Parameter[1];
				Parameter_EEPROM[0]=Voltage_Parameter[0]*10;
				EEPROM_Wirte(Parameter_EEPROM,0,1);
				//从参数界面退出时，将电压参数VP放大10倍后（VP*10），保存到E2PROM存储器（内部地址0），占用一个字节。
			}
		break;
		case 13://13：定义为"清零"按键，按下S13按键可将当前计数值清零。
			if(Seg_Show_Mode==2)
			{
				Key_Errors=0;
				Count=0;
//				Count++;
			}
			else
				Key_Errors++;
		break;
		case 16://S16："加"按键，按下电压参数VP增加0.5V；增加到5.00V后，再次按下返回0.00V。
			if(Seg_Show_Mode==1)
			{
				Key_Errors=0;
				Voltage_Parameter[1]+=0.5;
				if(Voltage_Parameter[1]==5.5)
					Voltage_Parameter[1]=0.0;
			}
			else
				Key_Errors++;
		break;
		case 17://S17："减"按键，按下电压参数VP减小0.5V；减小到0.00V后，再次按下返回5.00V。
			if(Seg_Show_Mode==1)
			{
				Key_Errors=0;
				Voltage_Parameter[1]-=0.5;
				if(Voltage_Parameter[1]==-0.5)
					Voltage_Parameter[1]=5.0;
			}
			else
				Key_Errors++;
		break;
			
		case 4:case 5:case 6:case 7:
		case 8:case 9:case 10:case 11:
		case 14:case 15:case 18:case 19:
			Key_Errors++;
		break;

	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	Voltage_AD_Data_Old=Voltage_AD_Data;
	Voltage_AD_Data=AD_Read(0x03)/51.0;
	if(Voltage_AD_Data_Old>Voltage_AD_Data && Voltage_AD_Data_Old>Voltage_Parameter[0] && Voltage_AD_Data<Voltage_Parameter[0])
		Count++;
	if(Voltage_AD_Data<Voltage_Parameter[0])
		Voltage_Below_Parameter_Flag=1;
	else
		Voltage_Below_Parameter_Flag=0;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned int)(Voltage_AD_Data)%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Voltage_AD_Data*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_AD_Data*100)%10;
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned int)(Voltage_Parameter[1])%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Voltage_Parameter[1]*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_Parameter[1]*100)%10;
		break;
		case 2:
			Seg_Point[5]=0;
			Seg_Buf[0]=13;
			Seg_Buf[1]=Count/1000000%10;
			Seg_Buf[2]=Count/100000%10;	
			Seg_Buf[3]=Count/10000%10;	
			Seg_Buf[4]=Count/1000%10;	
			Seg_Buf[5]=Count/100%10;	
			Seg_Buf[6]=Count/10%10;	
			Seg_Buf[7]=Count/1%10;	
			if(Seg_Buf[1]==0)
				Seg_Buf[1]=10;
			for(i=2;i<7;i++)
			{
				if(Seg_Buf[i]==0 && Seg_Buf[i-1]==10)
					Seg_Buf[i]=10;
			}
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Beep_Buf=Relay_Buf=0;
	Led_Buf[0]=(Time_5000ms_Flag==1)?1:0;
	Led_Buf[1]=(Count%2==1)?1:0;
	Led_Buf[2]=(Key_Errors>=3)?1:0;
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
	if(++Seg_Slow==80)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beep_Buf);
	Relay(Relay_Buf);
	
	if(Voltage_Below_Parameter_Flag==1)
	{
		if(++Time_5000ms==5000)
		{
			Time_5000ms=0;
			Time_5000ms_Flag=1;
		}
	}
	else
	{
		Time_5000ms=0;
		Time_5000ms_Flag=0;
	}
	
}
/*软件延时区域*/
void Delay1000ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 46;
	j = 153;
	k = 245;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	EEPROM_Read(Parameter_EEPROM,0,1);
	Voltage_Parameter[0]=Parameter_EEPROM[0]/10.0;
	Voltage_AD_Data=AD_Read(0x03)/51.0;	
	Delay1000ms();
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
