/*头文件引用区域*/
#include <main.H>
/*变量声明区域*/
u8 Key_Slow;
u8 Key_Val,Key_Old,Key_Down,Key_Up;
u8 Seg_Slow;
u8 Seg_Pos;
u8 Seg_Buf[8]={10,10,10,10,10,10,10,10};
u8 Seg_Point[8]={0,0,0,0,0,0,0,0};
u8 Led_Buf[8]={0,0,0,0,0,0,0,0};

u8 Seg_Show_Mode=0;//【0-电压界面】【1-测距界面】【2-参数界面】
bit Voltage_Parameter_Index=0;//参数选择位【0-上限】【1-下限】
bit UltraSound_Work_Mode=1;//超声波测距模式【0-停止测距】【1-启动连续测距】
bit L8_Flash_Flag;

u8 AD_Read_Data;
float Voltage_Value;
float Voltage_Set=3.0;
u8 UltraSound_Distance=456;//超声波测距值

float Voltage_Parameter[2]={4.5,0.5};//参数数组【0-上限】【1-下限】
float Voltage_Parameter_Set[2]={4.5,0.5};//参数设置数组【0-上限】【1-下限】


u8 Time_100ms=0;
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
		case 4://S4：“界面切换”按键，按下切换【0-电压】【1-测距】【2-参数】界面
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==2)//从【1-测距】界面进入【2-参数】界面
			{
				Voltage_Parameter_Index=0;//默认当前选择的是电压【0-上限】参数
				Voltage_Parameter_Set[0]=Voltage_Parameter[0];
				Voltage_Parameter_Set[1]=Voltage_Parameter[1];
			}
			if(Seg_Show_Mode==0)//从【2-参数】界面进入【0-电压】界面
			{
				if(Voltage_Parameter_Set[0]>Voltage_Parameter_Set[1])
				{
					Voltage_Parameter[0]=Voltage_Parameter_Set[0];
					Voltage_Parameter[1]=Voltage_Parameter_Set[1];
				}
			}
		break;
		case 5://S5：“参数选择”按键，切换选择【0-上限】或【1-下限】参数。仅在参数设置界面下有效
			if(Seg_Show_Mode==2)Voltage_Parameter_Index^=1;
		break;
		case 6://S6：“加”按键，每次按下，当前选择的电压参数增加0.5V。
			if(Seg_Show_Mode==2)
			{
				if(Voltage_Parameter_Index==0)//【0-上限】
				{
					Voltage_Parameter_Set[0]+=0.5;
					if(Voltage_Parameter_Set[0]==5.5)
						{Voltage_Parameter_Set[0]=0.5;}
				}
				else//【1-下限】
				{
					Voltage_Parameter_Set[1]+=0.5;
					if(Voltage_Parameter_Set[1]==5.5)
						{Voltage_Parameter_Set[1]=0.5;}
				}
			}
		break;
		case 7://S7：“减”按键，每次按下，当前选择的距离参数减少0.5V。
			if(Seg_Show_Mode==2)
			{
				if(Voltage_Parameter_Index==0)//【0-上限】
				{
					Voltage_Parameter_Set[0]-=0.5;
					if(Voltage_Parameter_Set[0]==0)
						{Voltage_Parameter_Set[0]=5.0;}
				}
				else//【1-下限】
				{
					Voltage_Parameter_Set[1]-=0.5;
					if(Voltage_Parameter_Set[1]==0)
						{Voltage_Parameter_Set[1]=5.0;}
				}
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;	
	
	UltraSound_Distance=Ultrasound_Distance_Get();
	
	AD_Read_Data=AD_Read(0x43);
	Voltage_Value=AD_Read_Data/51.0;
	//电位器RB2输出的电压值记为VAIN3，满足【电压【1-下限】参数<VAIN3<电压【0-上限】参数】时，启动连续超声测距功能；否则测距功能停止。
	if(Voltage_Parameter[1]<Voltage_Value && Voltage_Value<Voltage_Parameter[0])
		{UltraSound_Work_Mode=1;}
	else
		{UltraSound_Work_Mode=0;}
		
	if(UltraSound_Work_Mode==0)
		{Voltage_Set=0;}
	else
	{
		if(UltraSound_Distance<=20)
			{Voltage_Set=1.0;}
		else if(UltraSound_Distance>=80)
			{Voltage_Set=5.0;}
		else if(UltraSound_Distance>20 && UltraSound_Distance<80)
		{
			Voltage_Set=(float)(UltraSound_Distance-5)/15.0;
		}
	}
	DA_Write(Voltage_Set);
	
		
	
	switch(Seg_Show_Mode)
	{
		case 0://【0-电压】
			Seg_Point[3]=Seg_Point[6]=0;
			Seg_Buf[0]=11;//U
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			Seg_Buf[5]=(u16)(Voltage_Set)%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(u16)(Voltage_Set*10)%10;
			Seg_Buf[7]=(u16)(Voltage_Set*100)%10;
		break;
		case 1://【1-测距】
			Seg_Point[5]=0;
			Seg_Buf[0]=13;//V
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			if(UltraSound_Work_Mode==0)//超声波测距模式【0-停止测距】
				{Seg_Buf[5]=Seg_Buf[6]=Seg_Buf[7]=14;}//A
			else//超声波测距模式【1-启动连续测距】
			{
				if(UltraSound_Distance>=100 && UltraSound_Distance<=999)
				{
					Seg_Buf[5]=UltraSound_Distance/100%10;
					Seg_Buf[6]=UltraSound_Distance/10%10;
					Seg_Buf[7]=UltraSound_Distance/1%10;
				}
				else if(UltraSound_Distance>=10 && UltraSound_Distance<=99)
				{
					Seg_Buf[5]=10;
					Seg_Buf[6]=UltraSound_Distance/10%10;
					Seg_Buf[7]=UltraSound_Distance/1%10;
				}
				else if(UltraSound_Distance>=0 && UltraSound_Distance<=9)
				{
					Seg_Buf[5]=10;
					Seg_Buf[6]=10;
					Seg_Buf[7]=UltraSound_Distance/1%10;
				}
			}
		break;
		case 2://【2-参数】
			Seg_Buf[0]=12;//P
			Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[5]=10;
			Seg_Point[3]=1;
			Seg_Buf[3]=(u8)(Voltage_Parameter_Set[0])%10;
			Seg_Buf[4]=(u8)(Voltage_Parameter_Set[0]*10)%10;
			Seg_Point[6]=1;
			Seg_Buf[6]=(u8)(Voltage_Parameter_Set[1])%10;
			Seg_Buf[7]=(u8)(Voltage_Parameter_Set[1]*10)%10;

		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=(Seg_Show_Mode==0)?1:0;//L1【0-电压界面】
	Led_Buf[1]=(Seg_Show_Mode==1)?1:0;//L2【1-测距界面】
	Led_Buf[2]=(Seg_Show_Mode==2)?1:0;//L3【2-参数界面】
	
	Led_Buf[7]=(Time_100ms)?L8_Flash_Flag:0;//L8:启用连续测量功能时，指示灯L8以0.1秒为间隔闪烁，停止时L8熄灭。
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
	
	if(UltraSound_Work_Mode==1)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			L8_Flash_Flag^=1;
		}
	}
	else
		{Time_100ms=0;}
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	
	AD_Read_Data=AD_Read(0x43);
	Voltage_Value=AD_Read_Data/51.0;
	//电位器RB2输出的电压值记为VAIN3，满足【电压【1-下限】参数<VAIN3<电压【0-上限】参数】时，启动连续超声测距功能；否则测距功能停止。
	if(Voltage_Parameter[1]<Voltage_Value && Voltage_Value<Voltage_Parameter[0])
		{UltraSound_Work_Mode=1;}
	else
		{UltraSound_Work_Mode=0;}
		
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

