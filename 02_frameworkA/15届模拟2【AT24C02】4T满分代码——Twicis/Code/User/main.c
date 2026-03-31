/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char i,j;

idata unsigned char Seg_Show_Mode=0;//【0-密码】【1-存储】
pdata unsigned long int Password_True=12345678;

idata unsigned long int Input_Password;
idata unsigned int Input_Index=0;
idata unsigned char Key_Data_Location[10]={6,10,14,18,9,13,17,8,12,16};
idata bit Input_Success_Flag=0;
idata bit PW_Right_Flag=1;

idata unsigned char PW_Right_Count=0;

idata unsigned int Time_5000ms;
idata unsigned int Time_1000ms;
idata bit Time_1000ms_Flag;

idata unsigned char EEPROM_Save_Data[1];
/*按键控制区域*/
void Key_Proc()
{
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	
	if(Seg_Show_Mode==0)
	{
		for(i=0;i<10;i++)
		{
			if(Key_Down==Key_Data_Location[i] && Input_Index<8)
			{
				Input_Password=Input_Password*10+i;
				Input_Index++;
				Input_Success_Flag=1;
				break;
			}
		}
	}
	else
	{
		Input_Password=0;
		Input_Index=0;
		Input_Success_Flag=0;
	}

	
	
	switch(Key_Down)
	{
		case 4:
			if(Seg_Show_Mode==0)
			{
				if(Input_Password==Password_True)
				{
					Seg_Show_Mode=1;
					PW_Right_Count++;
					EEPROM_Save_Data[0]=PW_Right_Count;
					EEPROM_Write(EEPROM_Save_Data,0,1);
					PW_Right_Flag=1;
				}
				else
				{
					Input_Password=0;
					Input_Index=0;
					Input_Success_Flag=0;
					PW_Right_Flag=0;
				}
			}
		break;
		case 5:
			if(Seg_Show_Mode==0)
			{
				Input_Password=0;
				Input_Index=0;
				Input_Success_Flag=0;
			}
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	switch(Seg_Show_Mode)
	{
		case 0:
			if(Input_Success_Flag==1)
			{
				Seg_Buf[0]=Input_Index==8?Input_Password/10000000%10:10;
				Seg_Buf[1]=Input_Index>=7?Input_Password/1000000%10:10;
				Seg_Buf[2]=Input_Index>=6?Input_Password/100000%10:10;
				Seg_Buf[3]=Input_Index>=5?Input_Password/10000%10:10;
				Seg_Buf[4]=Input_Index>=4?Input_Password/1000%10:10;
				Seg_Buf[5]=Input_Index>=3?Input_Password/100%10:10;
				Seg_Buf[6]=Input_Index>=2?Input_Password/10%10:10;
				Seg_Buf[7]=Input_Index>=1?Input_Password/1%10:10;
			}
			else
			{
				for(j=0;j<8;j++)
					Seg_Buf[j]=11;
			}
		break;
		case 1:
			Seg_Buf[0]=12;
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=PW_Right_Count/100%10;
			Seg_Buf[6]=PW_Right_Count/10%10;
			Seg_Buf[7]=PW_Right_Count/1%10;
			for(j=5;j<7;j++)
			{
				if(Seg_Buf[j]!=0)break;
				Seg_Buf[j]=10;
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
	Led_Buf[7]=PW_Right_Flag==0?Time_1000ms_Flag:0;
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
	
	if(Seg_Show_Mode==1)
	{
		if(++Time_5000ms==5000)
		{
			Time_5000ms=0;
			Seg_Show_Mode=0;
		}
	}
	
	if(PW_Right_Flag==0)
	{
		if(++Time_1000ms==1000)
		{
			Time_1000ms=0;
			Time_1000ms_Flag^=1;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	EEPROM_Read(EEPROM_Save_Data,0,1);
	PW_Right_Count=EEPROM_Save_Data[0];
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
