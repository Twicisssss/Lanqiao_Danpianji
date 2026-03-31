/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Key.H>
#include <Seg.H>
#include <Led.H>
#include <iic.H>
/*变量声明区域*/
unsigned char Key_Slow;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

bit Led_System_State=0;//Led流转开始或停止状态
unsigned char Led_Run_Mode=0;//Led流转模式0123
unsigned char Led_Run_Pos=0;//Led流转位
unsigned char Seg_Show_Mode=0;//0-熄屏；1-选择；2-间隔
unsigned char Time_Brightness_Mode=0;//0-熄屏；1-亮度

unsigned char Led_Brightness;//1-4
unsigned char AD_Read_Data;
unsigned char Led_Go_Mode_Index;//选择
unsigned int Led_Interval_Time[4]={400,500,600,700};
unsigned int Led_Interval_Time_Set[4]={400,500,600,700};
unsigned char Led_Interval_Time_EEPROM[4]={4,5,6,7};

unsigned int Led_Light_Count=0;
unsigned int Time_800ms=0;
bit Time_800ms_Flag=0;
unsigned int Time_Xms=0;

/*按键控制区域*/
void Key_Proc()
{
	unsigned char i;
	if(Key_Slow)return;
	Key_Slow=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Old^Key_Val);
	Key_Up=~Key_Val&(Key_Old^Key_Val);
	Key_Old=Key_Val;
	
	if(Seg_Show_Mode==0)
	{
		if(Key_Old==4)
			Time_Brightness_Mode=1;
		else
			Time_Brightness_Mode=0;
	}
	
	
	switch(Key_Down)
	{
		case 7://开始或停止Led流转
			Led_System_State^=1;
		break;
		case 6://进入设置界面
			if(Seg_Show_Mode==0)
			{
				Led_Go_Mode_Index=0;
			}
			if(Seg_Show_Mode==1)
			{
				for(i=0;i<4;i++)
				{
					Led_Interval_Time_Set[i]=Led_Interval_Time[i];
				}
			}
			if(Seg_Show_Mode==2)
			{
				for(i=0;i<4;i++)
				{
					Led_Interval_Time[i]=Led_Interval_Time_Set[i];
					Led_Interval_Time_EEPROM[i]=Led_Interval_Time[i]/100;
				}
				EEPROM_Write(Led_Interval_Time_EEPROM,0,4);
			}
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
		break;
		case 5://加按键
			if(Seg_Show_Mode==1)
			{
				if(++Led_Go_Mode_Index==4)Led_Go_Mode_Index=0;
			}
			else if(Seg_Show_Mode==2)
			{
				Led_Interval_Time_Set[Led_Go_Mode_Index]+=100;
				if(Led_Interval_Time_Set[Led_Go_Mode_Index]>1200)Led_Interval_Time_Set[Led_Go_Mode_Index]=1200;
			}
		break;
		case 4://减按键
			if(Seg_Show_Mode==1)
			{
				if(--Led_Go_Mode_Index==255)Led_Go_Mode_Index=3;
			}
			else if(Seg_Show_Mode==2)
			{
				Led_Interval_Time_Set[Led_Go_Mode_Index]-=100;
				if(Led_Interval_Time_Set[Led_Go_Mode_Index]<400)Led_Interval_Time_Set[Led_Go_Mode_Index]=400;
			}
		break;
	}
}
/*数码管显示区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	AD_Read_Data=AD_Read(0x03);
	Led_Brightness=(AD_Read_Data/65)+1;
	
	if(Time_Brightness_Mode==0)
	{
		switch(Seg_Show_Mode)
		{
			case 0:
				Seg_Buf[0]=10;Seg_Buf[1]=10;Seg_Buf[2]=10;Seg_Buf[3]=10;Seg_Buf[4]=10;Seg_Buf[5]=10;Seg_Buf[6]=10;Seg_Buf[7]=10;
			break;
			case 1:
				Seg_Buf[0]=11;
				Seg_Buf[1]=Time_800ms_Flag?10:(Led_Go_Mode_Index+1);
				Seg_Buf[2]=11;
				Seg_Buf[3]=10;
			
				Seg_Buf[4]=Led_Interval_Time[Led_Go_Mode_Index]/1000%10;
				Seg_Buf[5]=Led_Interval_Time[Led_Go_Mode_Index]/100%10;
				Seg_Buf[6]=Led_Interval_Time[Led_Go_Mode_Index]/10%10;
				Seg_Buf[7]=Led_Interval_Time[Led_Go_Mode_Index]/1%10;
				if(Seg_Buf[4]==0)Seg_Buf[4]=10;
			break;
			case 2:
				Seg_Buf[0]=11;
				Seg_Buf[1]=Led_Go_Mode_Index+1;
				Seg_Buf[2]=11;
				Seg_Buf[3]=10;
			
				Seg_Buf[4]=Time_800ms_Flag?10:(Led_Interval_Time_Set[Led_Go_Mode_Index]/1000%10);
				Seg_Buf[5]=Time_800ms_Flag?10:(Led_Interval_Time_Set[Led_Go_Mode_Index]/100%10);
				Seg_Buf[6]=Time_800ms_Flag?10:(Led_Interval_Time_Set[Led_Go_Mode_Index]/10%10);
				Seg_Buf[7]=Time_800ms_Flag?10:(Led_Interval_Time_Set[Led_Go_Mode_Index]/1%10);
				if(Seg_Buf[4]==0)Seg_Buf[4]=10;
			break;
		}
	}
	if(Time_Brightness_Mode==1)
	{
		Seg_Buf[0]=10;Seg_Buf[1]=10;Seg_Buf[2]=10;Seg_Buf[3]=10;Seg_Buf[4]=10;Seg_Buf[5]=10;
		Seg_Buf[6]=11;
		Seg_Buf[7]=Led_Brightness;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	unsigned char i;
	if(Led_System_State==0)
	{
		if(Time_Xms==Led_Interval_Time[Led_Run_Mode])
		{
			Time_Xms=0;
			switch(Led_Run_Mode)
			{
				case 0://L0→L1→L2→L3→L4→L5→L6→L7
					if(++Led_Run_Pos==8)
					{
						Led_Run_Pos=7;
						Led_Run_Mode=1;
					}
				break;
				case 1://L7→L6→L5→L4→L3→L2→L1→L0
					if(--Led_Run_Pos==255)
					{
						Led_Run_Pos=7;
						Led_Run_Mode=2;
					}
				break;
				case 2://L07→L16→L25→L34
					Led_Run_Pos+=9;
					if(Led_Run_Pos>34)
					{
						Led_Run_Pos=34;
						Led_Run_Mode=3;
					}
				break;
				case 3://L34→L25→L16→L07
					Led_Run_Pos-=9;
					if(Led_Run_Pos>200)
					{
						Led_Run_Pos=0;
						Led_Run_Mode=0;
					}
				break;
			}
		}	
	}
	if(Led_Run_Mode==0||Led_Run_Mode==1)
	{
		for(i=0;i<8;i++)
		{
			Led_Buf[i]=(i==Led_Run_Pos);
		}
	}
	else if(Led_Run_Mode==2||Led_Run_Mode==3)
	{
		for(i=0;i<8;i++)
		{
			Led_Buf[i]=(i==(Led_Run_Pos/10)||i==(Led_Run_Pos%10));
		}
	}
	
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
	if(++Seg_Slow==500)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	
	if(++Led_Light_Count==13)Led_Light_Count=1;
	if(Led_Light_Count<=(Led_Brightness*3))
		Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	else
		Led_Disp(Seg_Pos,0);
	
	if(++Time_800ms==800)
	{
		Time_800ms=0;
		Time_800ms_Flag^=1;
	}
	if(Led_System_State==1)
		Time_Xms=0;
	else
		Time_Xms++;
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	EEPROM_Read(Led_Interval_Time_EEPROM,0,4);
	Led_Interval_Time[0]=Led_Interval_Time_Set[0]=Led_Interval_Time_EEPROM[0]*100;
	Led_Interval_Time[1]=Led_Interval_Time_Set[1]=Led_Interval_Time_EEPROM[1]*100;
	Led_Interval_Time[2]=Led_Interval_Time_Set[2]=Led_Interval_Time_EEPROM[2]*100;
	Led_Interval_Time[3]=Led_Interval_Time_Set[3]=Led_Interval_Time_EEPROM[3]*100;
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


