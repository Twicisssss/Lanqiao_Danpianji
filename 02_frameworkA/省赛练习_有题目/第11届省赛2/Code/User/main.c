/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <iic.H>
#include <onewire.H>
#include <intrins.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Slow;

unsigned char Seg_Show_Mode=0;//显示界面【0-数据显示界面】【1-参数设置界面】

float Temperature;//实时读取温度
unsigned char Temperature_Max=30;//温度上限参数0≤T_Max<100 
unsigned char Temperature_Min=20;//温度下限参数0≤T_Min<100 
unsigned char Temperature_Max_Set=30;//温度上限设置参数0≤T_Max<100 
unsigned char Temperature_Min_Set=20;//温度下限设置参数0≤T_Min<100 
unsigned char T_Max_Min_Set_Chose=1;//参数选择【0-温度上限参数TMax】和【1-温度下限参数TMin】
bit T_Change_Flag=1;//0-失败；1-成功
unsigned char AD_Read_Data;
unsigned char DA_Write_Data;
unsigned char Voltage_Set;//设置电压

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
		case 4://S4：“界面切换”按键，按下切换【0-数据显示界面】和【1-参数设置界面】。
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)//进入【1-参数设置界面】；离开【0-数据显示界面】
			{
				T_Max_Min_Set_Chose=1;
				Temperature_Max_Set=Temperature_Max;
				Temperature_Min_Set=Temperature_Min;
			}
			else if(Seg_Show_Mode==0)//进入【0-数据显示界面】；离开【1-参数设置界面】
			{
				T_Max_Min_Set_Chose=1;
				if(Temperature_Max_Set>=Temperature_Min_Set)
				{
					Led_Buf[3]=0;
					Temperature_Max=Temperature_Max_Set;
					Temperature_Min=Temperature_Min_Set;
				}
				else
					Led_Buf[3]=1;
			}
		break;
		case 5://S5：“参数切换”按键，按下切换选择【0-温度上限参数TMax】和【1-温度下限参数TMin】。
			if(++T_Max_Min_Set_Chose==2)T_Max_Min_Set_Chose=0;
		break;
		case 6://S6：“加”按键，按下则当前选择的温度参数增加1℃。
			if(Seg_Show_Mode==1)
			{
				if(T_Max_Min_Set_Chose==0)//【0-温度上限参数TMax】
				{
					if(++Temperature_Max_Set==100)
						{Temperature_Max_Set=99;}
				}
				else//【1-温度下限参数TMin】
				{					
					if(++Temperature_Min_Set==100)
						{Temperature_Min_Set=99;}
				}
			}
		break;
		case 7://S7：“减”按键，按下则当前选择的温度参数减少1℃。
			if(Seg_Show_Mode==1)
			{
				if(T_Max_Min_Set_Chose==0)//【0-温度上限参数TMax】
				{
					if(--Temperature_Max_Set==255)
						{Temperature_Max_Set=0;}
				}
				else//【1-温度下限参数TMin】
				{
					if(--Temperature_Min_Set==255)
						{Temperature_Min_Set=0;}
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
		
	Temperature=Temperature_Read();
	
	switch(Seg_Show_Mode)
	{
		case 0://【0-数据显示界面】		
			for(i=1;i<=5;i++)
				{Seg_Buf[i]=10;}
			Seg_Buf[0]=11;//C
				
			Seg_Buf[6]=(unsigned char)Temperature/10%10;
			Seg_Buf[7]=(unsigned char)Temperature/1%10;
		break;
		case 1://【1-参数设置界面】Temperature
			Seg_Buf[1]=10;Seg_Buf[2]=10;Seg_Buf[5]=10;
			Seg_Buf[0]=12;//P
		
			Seg_Buf[3]=Temperature_Max_Set/10%10;
			Seg_Buf[4]=Temperature_Max_Set/1%10;
		
			Seg_Buf[6]=Temperature_Min_Set/10%10;
			Seg_Buf[7]=Temperature_Min_Set/1%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
//	if(Led_Slow)return;
//	Led_Slow=1;

	if(Temperature>Temperature_Max)
	{
		Led_Buf[0]=1;
		Voltage_Set=4;
		Led_Buf[1]=0;
		Led_Buf[2]=0;
	}
	else if(Temperature>=Temperature_Min&&Temperature<=Temperature_Max)
	{
		Led_Buf[1]=1;
		Voltage_Set=3;
		Led_Buf[0]=0;
		Led_Buf[2]=0;
	}
	else if(Temperature<Temperature_Min)
	{
		Led_Buf[2]=1;
		Voltage_Set=2;
		Led_Buf[0]=0;
		Led_Buf[1]=0;
	}
	DA_Write(Voltage_Set*51);
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
	if(++Seg_Slow==200)Seg_Slow=0;
	if(++Led_Slow==200)Led_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
}
/*延时函数*/
void Delay10ms(void)	//@12.000MHz
{
	unsigned char data i, j;

	i = 117;
	j = 184;
	do
	{
		while (--j);
	} while (--i);
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer0_Init();
	Delay10ms();
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
