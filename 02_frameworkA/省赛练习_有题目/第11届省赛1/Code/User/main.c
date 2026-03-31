/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Seg_Slow;
unsigned char AD_Read_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};


unsigned char Seg_Show_Mode=0;//【0-数据】【1-参数】【2-计数】

unsigned char AD_Read_Data;
float Voltage_VAIN3_Old;//上一次AIN3通道采集到的电压值VAIN3
float Voltage_VAIN3;//AIN3通道采集到的电压值VAIN3
float Voltage_Parameter;//电压参数
float Voltage_Parameter_Set;//电压参数设置值
unsigned char EEPROM_Save_Data[1]={0};//EEPROM电压参数存储值
unsigned char EEPROM_Read_Data[1]={0};//EEPROM电压参数读取值
unsigned long int V_Count=0;//计数
bit VAIN3_Low_5s_Flag;//VAIN3 < VP的状态持续时间超过5秒标志位
unsigned char Key_Error_Count=0;

unsigned int Time_5000ms=0;
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
		case 12://S12：“显示界面切换”按键，按下切换【0-数据】【1-参数】【2-计数】界面
			Key_Error_Count=0;
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)//进入【1-参数】界面
				{Voltage_Parameter_Set=Voltage_Parameter;}
			else if(Seg_Show_Mode==2)//离开【1-参数】界面
			{
				Voltage_Parameter=Voltage_Parameter_Set;
				EEPROM_Save_Data[0]=(unsigned char)(Voltage_Parameter*10);	/*从参数界面退出时，将电压参数VP放大10倍后（VP*10）*/
				EEPROM_Write(EEPROM_Save_Data,0,1);							/*保存到E2PROM存储器（内部地址0），占用一个字节。*/
			}
		break;
		case 13://S13：“清零”按键，按下可将当前计数值清零。仅在【2-计数】界面有效。
			if(Seg_Show_Mode==2)
			{
				Key_Error_Count=0;
				V_Count=0;
			}
			else
			{
				Key_Error_Count++;
			}
		break;
		case 16://S16：“加”按键，按下电压参数VP增加0.5V；增加到5.00V再按返回0.00V。仅在【1-参数】设置界面有效。
			if(Seg_Show_Mode==1)
			{
				Key_Error_Count=0;
				Voltage_Parameter_Set+=0.5;
				if(Voltage_Parameter_Set==5.5)
					Voltage_Parameter_Set=0;
			}
			else
			{
				Key_Error_Count++;
			}
		break;
		case 17://S17：“减”按键，按下电压参数VP减小0.5V；减小到0.00V再按返回5.00V。仅在【1-参数】设置界面有效。
			if(Seg_Show_Mode==1)
			{
				Key_Error_Count=0;
				Voltage_Parameter_Set-=0.5;
				if(Voltage_Parameter_Set==-0.5)
					Voltage_Parameter_Set=5;
			}
			else
			{
				Key_Error_Count++;
			}
		break;
			
		case 4:case 5:case 6:case 7:
		case 8:case 9:case 10:case 11:
					case 14:case 15:
					case 18:case 19:
			Key_Error_Count++;
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i;	
	
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	AD_Read_Data=AD_Read(0x43);
	Voltage_VAIN3=AD_Read_Data/51.0;
	
		
	switch(Seg_Show_Mode)
	{
		case 0://【0-数据】AD_Read_Data
			for(i=1;i<=4;i++)
				{Seg_Buf[i]=10;}
			Seg_Buf[0]=11;//U
			Seg_Buf[5]=(unsigned int)Voltage_VAIN3/1%10;
			Seg_Point[5]=1;//小数点
			Seg_Buf[6]=(unsigned int)(Voltage_VAIN3*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_VAIN3*100)%10;
		break;
		case 1://【1-参数】
			for(i=1;i<=4;i++)
				{Seg_Buf[i]=10;}
			Seg_Buf[0]=12;//P
			Seg_Buf[5]=(unsigned int)Voltage_Parameter_Set/1%10;
			Seg_Point[5]=1;//小数点
			Seg_Buf[6]=(unsigned int)(Voltage_Parameter_Set*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_Parameter_Set*100)%10;
		break;
		case 2://【2-计数】
			Seg_Point[5]=0;//清除小数点
			Seg_Buf[0]=13;//N
		Seg_Buf[1]=(V_Count/1000000%10)?(V_Count/1000000%10):10;
		Seg_Buf[2]=(V_Count/100000%10)?(V_Count/100000%10):10;
		Seg_Buf[3]=(V_Count/10000%10)?(V_Count/10000%10):10;
		Seg_Buf[4]=(V_Count/1000%10)?(V_Count/1000%10):10;
		Seg_Buf[5]=(V_Count/100%10)?(V_Count/100%10):10;
		Seg_Buf[6]=(V_Count/10%10)?(V_Count/10%10):10;
		Seg_Buf[7]=(V_Count/1%10)?(V_Count/1%10):10;
//		Seg_Buf[7]=V_Count/1%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=VAIN3_Low_5s_Flag?1:0;	//1) 指示灯L1：当VAIN3 < VP的状态持续时间超过5秒时，	L1点亮，否则熄灭。 
	Led_Buf[1]=(V_Count%2)?1:0;			//2) 指示灯L2：当前计数值为奇数时，					L2点亮，否则熄灭。 
	Led_Buf[2]=(Key_Error_Count>=3)?1:0;//3) 指示灯L3：连续3次以上（含3次）的无效按键操作时，	L3点亮，直到出现有效的按键操作，L3熄灭。
}
/*定时器区域*/
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
	
	if(Voltage_VAIN3_Old>Voltage_VAIN3 && Voltage_VAIN3_Old>Voltage_Parameter && Voltage_VAIN3<=Voltage_Parameter)
		{V_Count++;}
	Voltage_VAIN3_Old=Voltage_VAIN3;
		
		
	if(Voltage_VAIN3<Voltage_Parameter)
	{
		if(++Time_5000ms==5000)
		{
			Time_5000ms=0;
			VAIN3_Low_5s_Flag=1;
		}
	}
	else if(Voltage_VAIN3>=Voltage_Parameter)
	{
		Time_5000ms=0;
		VAIN3_Low_5s_Flag=0;
	}
}
/*初始化区域*/
void Init_Proc()
{
	EEPROM_Read(EEPROM_Read_Data,0,1);
	Voltage_Parameter=EEPROM_Read_Data[0]/10.0;
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
