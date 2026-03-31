/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
#include <iic.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Seg_Show_Mode=0;//【0-温度】【1-DAC输出控制】【2-校准值】
float Temperature_DS18B20;
char Temperature_Correction=0;
char Temperature_Correction_Set=0;
unsigned char Temperature;
unsigned char Temperature_Near;

unsigned char Temperature_DAC_Voltage;
unsigned char DAC_Value=100;
unsigned char DAC_Value_Set=100;
bit DAC_Output_Mode=0;//DAC输出控制模式，【0-温度控制】【1-手动控制】两种DAC 输出模式
bit Relay_Work_Mode=1;//继电器状态，【0-锁定】【1-解锁】
unsigned int S13_Time_1500ms;
bit S13_Start_Flag;
bit S13_Keep_Flag;

unsigned int L2_Time_2000ms=0;
bit L2_Start_Flag=0;
bit L2_Keep_Flag=0;
unsigned int L3_Time_2000ms=0;
bit L3_Start_Flag=0;
bit L3_Keep_Flag=0;
unsigned int L4_Time_3000ms=0;
unsigned int L4_Flash_200ms=0;
bit L4_Start_Flag=0;
bit L4_Keep_Flag=0;
bit L4_Flash_Flag=0;
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	
	if(Key_Old==13)
	{
		S13_Start_Flag=1;
	}
	if(Key_Up==13 && S13_Keep_Flag==0)
	{
		S13_Start_Flag=0;
		S13_Time_1500ms=0;
		S13_Keep_Flag=0;
	}
	else if(Key_Up==13 && S13_Keep_Flag==1)
	{
		S13_Start_Flag=0;
		S13_Time_1500ms=0;
		S13_Keep_Flag=0;
		
		Relay_Work_Mode^=1;//② 长按S13 按键超过1.5 秒后松开，触发继电器“锁定”状态切换功能。	
	}
	
	
	
	switch(Key_Down)
	{
		case 12://S12：界面切换按键，按下切换显示【0-温度】【1-DAC输出控制】【2-校准值】界面。
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)
			{
				DAC_Value_Set=DAC_Value;
			}
			else if(Seg_Show_Mode==2)
			{
				DAC_Value=DAC_Value_Set;
				Temperature_Correction_Set=Temperature_Correction;
			}
			else if(Seg_Show_Mode==0)
			{
				Temperature_Correction=Temperature_Correction_Set;
			}
		break;
		case 13://S13：定义为模式/锁定按键。 
			DAC_Output_Mode^=1;//① 短按S13按键，切换DAC输出控制模式。
		break;
		case 16://S16：定义为“加”按键。
			if(Seg_Show_Mode==1)		//【1-DAC输出控制】界面，按下按键，DAC数字量加5。
			{
				DAC_Value_Set+=5;
				if(DAC_Value_Set==4)DAC_Value_Set=255;
			}
			else if(Seg_Show_Mode==2)	//【2-校准值】界面，按下按键，温度校准值加1。
			{
				Temperature_Correction_Set++;
				if(Temperature_Correction_Set==10)Temperature_Correction_Set=9;
			}
			else if(Seg_Show_Mode==0)	//【0-温度】界面且当前为【解锁】状态，按下按键，打开继电器。
			{
				if(Relay_Work_Mode==1)
					Relay(1);
			}
		break;
		case 17://S17：定义为“减”按键。
			if(Seg_Show_Mode==1)		//【1-DAC输出控制】界面，按下按键，DAC数字量减5。
			{
				DAC_Value_Set-=5;
				if(DAC_Value_Set==251)DAC_Value_Set=0;
			}
			else if(Seg_Show_Mode==2)	//【2-校准值】界面，按下按键，温度校准值减1。
			{
				Temperature_Correction_Set--;
				if(Temperature_Correction_Set==-10)Temperature_Correction_Set=-9;
			}
			else if(Seg_Show_Mode==0)	//【0-温度】界面且当前为【解锁】状态，按下按键，关闭继电器。
			{
				if(Relay_Work_Mode==1)
					Relay(0);
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature_Near=Temperature;
	Temperature_DS18B20=Temperature_Read();
	Temperature=(unsigned char)Temperature_DS18B20+Temperature_Correction;
	
	if(Temperature>=10 && Temperature<=40)
		Temperature_DAC_Voltage=10*Temperature-98;
	else if(Temperature<10)
		Temperature_DAC_Voltage=2;
	else if(Temperature>40)
		Temperature_DAC_Voltage=5;
	
	if(DAC_Output_Mode==0)
	{
		DA_Write(Temperature_DAC_Voltage*51);
	}
	else if(DAC_Output_Mode==1)
	{
		DA_Write(DAC_Value);
	}
	
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[0]=11;//C
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=(unsigned char)Temperature/10%10;
			Seg_Buf[7]=(unsigned char)Temperature/1%10;
		break;
		case 1:
			Seg_Buf[0]=12;//A
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(DAC_Value_Set>=100)
			{
				Seg_Buf[5]=DAC_Value_Set/100%10;
				Seg_Buf[6]=DAC_Value_Set/10%10;
				Seg_Buf[7]=DAC_Value_Set/1%10;
			}
			else if(DAC_Value_Set<100 && DAC_Value_Set>=10)
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=DAC_Value_Set/10%10;
				Seg_Buf[7]=DAC_Value_Set/1%10;
			}
			else if(DAC_Value_Set<10 && DAC_Value_Set>=0)
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=10;
				Seg_Buf[7]=DAC_Value_Set/1%10;
			}
		break;
		case 2:
			Seg_Buf[0]=13;//P
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
			if(Temperature_Correction_Set>=0)
			{
				Seg_Buf[6]=10;
				Seg_Buf[7]=Temperature_Correction_Set%10;
			}
			else
			{
				Seg_Buf[6]=14;
				Seg_Buf[7]=-Temperature_Correction_Set%10;
			}
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	if(Temperature_Near<Temperature)
	{
		L2_Start_Flag=1;
		L3_Start_Flag=0;
	}
	else
	{
		L3_Start_Flag=1;
		L2_Start_Flag=0;
	}

	if((Temperature>Temperature_Near && Temperature-Temperature_Near>=1) || (Temperature_Near>Temperature && Temperature_Near-Temperature>=1))
	{
		L4_Start_Flag=1;
	}
	
	Led_Buf[0]=(DAC_Output_Mode==0)?1:0;
	
	Led_Buf[1]=(L2_Start_Flag==1)?1:0;
	Led_Buf[2]=(L3_Start_Flag==1)?1:0;
	Led_Buf[3]=(L4_Start_Flag==1)?L4_Flash_Flag:0;
	
	
	Led_Buf[7]=(Relay_Work_Mode==1)?1:0;
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
	
	
	if(S13_Start_Flag==1)
	{
		if(++S13_Time_1500ms==1500)
		{
			S13_Time_1500ms=0;
			S13_Keep_Flag=1;
		}
	}
	else
	{
		S13_Time_1500ms=0;
		S13_Keep_Flag=0;	
	}



	
	if(L2_Start_Flag==1)
	{
		if(++L2_Time_2000ms==2000)
		{
			L2_Time_2000ms=0;
			L2_Keep_Flag=1;
			L2_Start_Flag=0;
		}
	}
	else
	{
		L2_Time_2000ms=0;
		L2_Keep_Flag=0;	
	}

	if(L3_Start_Flag==1)
	{
		if(++L3_Time_2000ms==2000)
		{
			L3_Time_2000ms=0;
			L3_Keep_Flag=1;
			L3_Start_Flag=0;
		}
	}
	else
	{
		L3_Time_2000ms=0;
		L3_Keep_Flag=0;	
	}

	
	if(L4_Start_Flag==1)
	{
		if(++L4_Time_3000ms==3000)
		{
			L4_Time_3000ms=0;
			L4_Keep_Flag=1;
			L4_Start_Flag=0;
		}
		if(++L4_Flash_200ms==200)
		{
			 L4_Flash_Flag^=1;
		}
	}
	else
	{
		L4_Time_3000ms=0;
		L4_Keep_Flag=0;	
	}

}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Beeper(0);
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
