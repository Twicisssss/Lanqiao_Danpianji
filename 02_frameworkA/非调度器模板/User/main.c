/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <ds1302.H>
#include <onewire.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};

idata unsigned char Seg_Show_Mode=0;//【0-时间】【1-温度】【2-AD读取】
idata unsigned char AD_Index=0;//【0-AIN1光敏电阻】【1-AIN3滑动电阻】

idata unsigned char Rtc[3]={0x23,0x59,0x55};//时分秒
idata float Temperature;
idata unsigned char Voltage_AIN1_Light;
idata unsigned char Voltage_AIN3_RB2;
idata unsigned char EEPROM_Save_Data[6];

idata unsigned int Time_500ms;
idata unsigned int Time_500ms_Flag;

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
			if(Seg_Show_Mode==2)AD_Index=0;
		break;
		case 5:
			if(Seg_Show_Mode==2)
			{
				if(++AD_Index==2)AD_Index=0;
			}
		break;
		case 8:
			EEPROM_Save_Data[0]=Rtc[0];
			EEPROM_Save_Data[1]=Rtc[1];
			EEPROM_Save_Data[2]=Rtc[2];
			EEPROM_Save_Data[3]=(unsigned char)Temperature;
			EEPROM_Save_Data[4]=Voltage_AIN1_Light;
			EEPROM_Save_Data[5]=Voltage_AIN3_RB2;
			EEPROM_Write(EEPROM_Save_Data,0,6);
		break;
		case 9:
			
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{	
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	
	RTC_Read(Rtc);//ds1302:ds1302
	
	Temperature=Temperature_Read();//onewire:ds18b20
	
	Voltage_AIN1_Light=AD_Read(0x03);//iic:PCF8591
	Voltage_AIN3_RB2=AD_Read(0x01);//iic:PCF8591
	
	
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=Rtc[0]/16%16;
			Seg_Buf[1]=Rtc[0]%16;
			Seg_Buf[2]=11;
			Seg_Buf[3]=Rtc[1]/16%16;
			Seg_Buf[4]=Rtc[1]%16;
			Seg_Buf[5]=11;
			Seg_Buf[6]=Rtc[2]/16%16;
			Seg_Buf[7]=Rtc[2]%16;
		break;
		case 1:
			Seg_Buf[0]=1;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=10;
			Seg_Buf[4]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[5]=(unsigned char)(Temperature)/1%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(Temperature*10)%10;
			Seg_Buf[7]=(unsigned int)(Temperature*100)%10;
		break;
		case 2:
			Seg_Point[5]=0;
			Seg_Buf[0]=2;
			if(AD_Index==0)
			{
				Seg_Buf[1]=10;
				Seg_Buf[2]=10;
				Seg_Buf[3]=1;
				Seg_Buf[4]=10;
				Seg_Buf[5]=Voltage_AIN1_Light/100%10;
				Seg_Buf[6]=Voltage_AIN1_Light/10%10;
				Seg_Buf[7]=Voltage_AIN1_Light/1%10;
			}
			else
			{
				Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=10;
				Seg_Buf[1]=10;
				Seg_Buf[2]=10;
				Seg_Buf[3]=3;
				Seg_Buf[4]=10;
				Seg_Buf[5]=Voltage_AIN3_RB2/100%10;
				Seg_Buf[6]=Voltage_AIN3_RB2/10%10;
				Seg_Buf[7]=Voltage_AIN3_RB2/1%10;
			}
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Beeper_Buf=Relay_Buf=0;
	Led_Buf[0]=1?Time_500ms_Flag:0;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	
	if(++Time_500ms==500)
	{
		Time_500ms=0;
		Time_500ms_Flag^=1;
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
	while(Temperature_Read()==85);//onewire:ds18b20
	RTC_Set(Rtc);//ds1302:ds1302
	EEPROM_Read(EEPROM_Save_Data,0,6);//iic:AT24C02
	
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
