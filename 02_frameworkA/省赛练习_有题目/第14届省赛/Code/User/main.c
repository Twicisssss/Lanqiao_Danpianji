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
idata unsigned char Key_Slow;
idata unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
idata unsigned char Seg_Slow;
idata unsigned char Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Seg_Show_Mode=0;//【0-时间】【1-回显】【2-参数】界面
unsigned char Echo_Show_Mode=0;//【0-温度回显】【1-湿度回显】【2-时间回显】子界面

unsigned char Rtc[3]={0x23,0x59,0x50};
float Temperature;
float Humidity;
float T_H_Near[2];
float T_H_Near_Before[2];
bit Humidity_Valid=1;
bit T_H_All_Up_Flag=0;

unsigned char Echo_Temperature_Max=0;
unsigned char Echo_Temperature_Average=0;
unsigned char Echo_Humidity_Max=0;
unsigned char Echo_Humidity_Average=0;
unsigned char Echo_Time_Near[2]={0x00,0x00};
unsigned char Acquisition_Number=0;

float Temperature_Parameter=30.0;
float Temperature_Parameter_Set=30.0;

unsigned char Light_Data;
unsigned char Voltage_Data;
bit Surrounding_Brightness=1;
bit Surrounding_Brightness_Before=1;
bit Data_Acquisition_Function=0;
unsigned int Acquisition_Time_3000ms;

bit S9_Start_Flag=0;
bit S9_Keep_Flag=0;
unsigned int S9_Time_2000ms;

unsigned int Ne555_Time_1000ms;
unsigned int Frequency;
bit Ne555_Finish_Flag=0;

bit Temperature_Over;
bit L4_Flash_Flag;
unsigned int L4_Time_100ms;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Seg_Show_Mode==1 && Echo_Show_Mode==2)//【2-时间回显】子界面下，长按S9超过2秒后松开，清除所有已记录的数据，触发次数重置为0。
	{
		if(Key_Old==9)
		{
			S9_Start_Flag=1;
		}
		if(Key_Up==9 && S9_Keep_Flag==0)
		{
			S9_Start_Flag=0;
			S9_Time_2000ms=0;
			S9_Keep_Flag=0;
		}
		else if(Key_Up==9 && S9_Keep_Flag==1)
		{
			S9_Start_Flag=0;
			S9_Time_2000ms=0;
			S9_Keep_Flag=0;

			Echo_Temperature_Max=0;
			Echo_Temperature_Average=0;
			Echo_Humidity_Max=0;
			Echo_Humidity_Average=0;
			Echo_Time_Near[0]=0x00;
			Echo_Time_Near[1]=0x00;
			Acquisition_Number=0;
		}
	}
	
	
	switch(Key_Down)
	{
		case 4://S4：“界面切换”按键，按下切换【0-时间】【1-回显】【2-参数】界面。 
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)Echo_Show_Mode=0;//从【0-时间】界面切换到【1-回显】界面时，处于【0-温度回显】子界面。
			if(Seg_Show_Mode==2)Temperature_Parameter_Set=Temperature_Parameter;//从【1-回显】界面切换到【2-参数】界面
			if(Seg_Show_Mode==0)Temperature_Parameter=Temperature_Parameter_Set;//从【2-参数】界面切换到【0-时间】界面
		break;
		case 5://S5：“回显切换”按键，按下切换【0-温度回显】【1-湿度回显】【2-时间回显】子界面。按键S5仅在回显界面有效。 
			if(Seg_Show_Mode==1)//按键S5仅在【1-回显】界面有效。 
			{
				if(++Echo_Show_Mode==3)Echo_Show_Mode=0;
			}
		break;
		case 8://S8：“加”按键，【2-参数】界面下，按下温度参数值加1。
			if(Seg_Show_Mode==2)
			{
				if(++Temperature_Parameter_Set==100)Temperature_Parameter_Set=99;
			}
		break;
		case 9://S9：“减”按键，【2-参数】界面下，按下温度参数值减1。
			if(Seg_Show_Mode==2)
			{
				if(--Temperature_Parameter_Set==-1)Temperature_Parameter_Set=0;
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	/*时钟*/
	Rtc_Read(Rtc);
	
	/*温度*/
	Temperature=Temperature_Read();
	
	/*频率转湿度：若测量到的频率不在【200Hz–2000Hz】范围内，认为是无效数据。*/
	if(Frequency>=200 && Frequency<=2000)
		Humidity=(float)(Frequency*2+50)/45;
	else
		Humidity=0;
		
	/*AD:光敏电阻和固定电阻上的分压结果*/
	Voltage_Data=AD_Read(0x01);
	Light_Data=AD_Read(0x03);//正常值为075
	Surrounding_Brightness_Before=Surrounding_Brightness;
	if(Light_Data>=50)
		Surrounding_Brightness=1;//“未挡光”认为是【1-亮】状态。
	else
		Surrounding_Brightness=0;//“已挡光”认为是【0-暗】状态
	if(Surrounding_Brightness_Before==1 && Surrounding_Brightness==0 && Data_Acquisition_Function==0 && Ne555_Finish_Flag==1)//当检测到环境从“亮”状态切换到“暗”状态时
	{
		Data_Acquisition_Function=1;//触发一次温度、湿度数据采集功能。
		Echo_Time_Near[0]=Rtc[0];
		Echo_Time_Near[1]=Rtc[1];
		if(Echo_Temperature_Max<Temperature)
			Echo_Temperature_Max=Temperature;
		if(Echo_Humidity_Max<Humidity)
			Echo_Humidity_Max=Humidity;
		Echo_Temperature_Average=(Echo_Temperature_Average*Acquisition_Number+Temperature)/(Acquisition_Number+1);
		Echo_Humidity_Average=(Echo_Humidity_Average*Acquisition_Number+Humidity)/(Acquisition_Number+1);
		Acquisition_Number++;
		
		if(Acquisition_Number>=2)
		{
			T_H_Near_Before[0]=T_H_Near[0];
			T_H_Near_Before[1]=T_H_Near[1];
		}
		T_H_Near[0]=Temperature;
		T_H_Near[1]=Humidity;
		if(Acquisition_Number>=2 && T_H_Near_Before[0]<T_H_Near[0] && T_H_Near_Before[1]<T_H_Near[1])
			T_H_All_Up_Flag=1;
		else
			T_H_All_Up_Flag=0;
		if(T_H_Near[0]>Temperature_Parameter)
			Temperature_Over=1;
		else
			Temperature_Over=0;
		
		if(T_H_Near[1]==0)
			Humidity_Valid=0;
		else
			Humidity_Valid=1;


	}
		
		
	if(Ne555_Finish_Flag==1)
	{		
		if(Data_Acquisition_Function==0)
		{
			switch(Seg_Show_Mode)
			{
				case 0://【0-时间】界面
					Seg_Buf[0]=Rtc[0]/16;
					Seg_Buf[1]=Rtc[0]%16;
					Seg_Buf[2]=11;//-
					Seg_Buf[3]=Rtc[1]/16;
					Seg_Buf[4]=Rtc[1]%16;
					Seg_Buf[5]=11;//-
					Seg_Buf[6]=Rtc[2]/16;
					Seg_Buf[7]=Rtc[2]%16;
				break;
				case 1://【1-回显】界面
					if(Echo_Show_Mode==0)
					{
						if(Acquisition_Number>=1)
						{
							Seg_Buf[0]=12;//C
							Seg_Buf[1]=10;
							Seg_Buf[2]=Echo_Temperature_Max/10%10;
							Seg_Buf[3]=Echo_Temperature_Max/1%10;//最大温度
							Seg_Buf[4]=11;//-
							Seg_Buf[5]=(unsigned int)(Echo_Temperature_Average)/10%10;
							Seg_Buf[6]=(unsigned int)(Echo_Temperature_Average)%10;
							Seg_Point[6]=1;
							Seg_Buf[7]=(unsigned int)(Echo_Temperature_Average*10)%10;//平均温度
						}
						else
						{
							Seg_Point[6]=0;
							Seg_Buf[0]=12;//C
							Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
						}
					}
					else if(Echo_Show_Mode==1)
					{
						if(Acquisition_Number>=1)
						{
							Seg_Buf[0]=13;//H
							Seg_Buf[1]=10;
							Seg_Buf[2]=Echo_Humidity_Max/10%10;
							Seg_Buf[3]=Echo_Humidity_Max/1%10;//最大湿度
							Seg_Buf[4]=11;//-
							Seg_Buf[5]=(unsigned int)(Echo_Humidity_Average)/10%10;
							Seg_Buf[6]=(unsigned int)(Echo_Humidity_Average)%10;
							Seg_Point[6]=1;
							Seg_Buf[7]=(unsigned int)(Echo_Humidity_Average*10)%10;//平均湿度
						}
						else
						{
							Seg_Point[6]=0;
							Seg_Buf[0]=13;//H
							Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
						}
					}
					else if(Echo_Show_Mode==2)
					{
						if(Acquisition_Number>=1)
						{
							Seg_Point[6]=0;
							Seg_Buf[0]=14;//F
							Seg_Buf[1]=Acquisition_Number/10%10;
							Seg_Buf[2]=Acquisition_Number/1%10;//触发次数：采集功能累计触发的次数，长度不足2位时左侧补0。
							Seg_Buf[3]=Echo_Time_Near[0]/16;
							Seg_Buf[4]=Echo_Time_Near[0]%16;//触发时间[时]：最近一次触发数据采集功能的时间。
							Seg_Buf[5]=11;//-
							Seg_Buf[6]=Echo_Time_Near[1]/16;
							Seg_Buf[7]=Echo_Time_Near[1]%16;//触发时间[分]：最近一次触发数据采集功能的时间。
						}
						else
						{
							Seg_Point[6]=0;
							Seg_Buf[0]=14;//F
							Seg_Buf[1]=Acquisition_Number/10%10;
							Seg_Buf[2]=Acquisition_Number/1%10;//触发次数00 
							Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=10;
						}
					}
				break;
				case 2://【2-参数】界面
					Seg_Point[6]=0;
					Seg_Buf[0]=15;//P
					Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
					Seg_Buf[6]=(unsigned char)Temperature_Parameter_Set/10%10;
					Seg_Buf[7]=(unsigned char)Temperature_Parameter_Set/1%10;//温度参数
//					Seg_Buf[1]=Voltage_Data/100%10;
//					Seg_Buf[2]=Voltage_Data/10%10;
//					Seg_Buf[3]=Voltage_Data/1%10;
//				Seg_Buf[4]=10;
//					Seg_Buf[5]=Light_Data/100%10;
//					Seg_Buf[6]=Light_Data/10%10;
//					Seg_Buf[7]=Light_Data/1%10;
				break;
			}
		}
		else
		{
					Seg_Point[6]=0;
					Seg_Buf[0]=16;//P
					Seg_Buf[1]=10;
					Seg_Buf[2]=10;
					Seg_Buf[3]=(unsigned char)(Temperature)/10%10;
					Seg_Buf[4]=(unsigned char)(Temperature)%10;
					Seg_Buf[5]=11;
					Seg_Buf[6]=(Humidity==0)?17:(unsigned char)(Humidity)/10%10;
					Seg_Buf[7]=(Humidity==0)?17:(unsigned char)(Humidity)%10;
		}
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=(Data_Acquisition_Function==0 && Seg_Show_Mode==0)?1:0;
	Led_Buf[1]=(Data_Acquisition_Function==0 && Seg_Show_Mode==1)?1:0;
	Led_Buf[2]=(Data_Acquisition_Function==1)?1:0;
	
	Led_Buf[3]=(Temperature_Over==1)?L4_Flash_Flag:0;
	Led_Buf[4]=(Humidity_Valid==0)?1:0;
	Led_Buf[5]=(T_H_All_Up_Flag==1)?1:0;
}
/*定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
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
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	if(++Ne555_Time_1000ms==1000)
	{
		Ne555_Finish_Flag=1;
		Ne555_Time_1000ms=0;
		Frequency=(TH0<<8)|TL0;
		TH0=TL0=0;
	}
	
	
	if(Data_Acquisition_Function==1)
	{
		if(++Acquisition_Time_3000ms==3000)
		{
			Data_Acquisition_Function=0;
			Acquisition_Time_3000ms=0;
		}
	}
		
	
	if(S9_Start_Flag==1 && S9_Keep_Flag==0)
	{
		if(++S9_Time_2000ms==2000)
		{
			S9_Time_2000ms=0;
			S9_Keep_Flag=1;
		}
	}
	
	
	if(Temperature_Over==1)
	{
		if(++L4_Time_100ms==100)
		{
			L4_Time_100ms=0;
			L4_Flash_Flag^=1;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Rtc_Set(Rtc);
	
	Timer0_Init();
	Timer1_Init();
}
/*主函数区域*/
void main()
{
	Init_Proc();
	while(1)
	{
		if(Data_Acquisition_Function==0)
			Key_Proc();
		Seg_Proc();
		Led_Proc();		
	}
}

