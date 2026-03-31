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

unsigned char Seg_Enable_Mode=0;//0-启用；1-禁用
unsigned char Led_Enable_Mode=0;//0-启用；1-禁用
unsigned char Seg_Show_Mode=1;//0-频率；1-电压
unsigned char DAC_Voltage_OutPut_Mode=1;//0-VRB2；1-固定2.0V


unsigned int Frequency;
unsigned int Time_1000ms=0;
unsigned char AD_RB2_U=0;
float Voltage=0;

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
		case 4://S4：“显示界面切换”按键，按下切换【0-频率界面；1-电压界面】
			if(++Seg_Show_Mode==2)Seg_Show_Mode=0;
		break;
		case 5://S5：“DAC输出模式切换”按键，按下切换【0-DAC输出电压跟随RB2电压变化；1-输出固定电压2.0V，不随RB2电压变化】
			if(++DAC_Voltage_OutPut_Mode==2)DAC_Voltage_OutPut_Mode=0;
		break;
		case 6://S6：“LED指示灯功能控制”按键，按下切换【0-打开LED指示功能；1-关闭LED指示功能】
			if(++Led_Enable_Mode==2)Led_Enable_Mode=0;
		break;
		case 7://S7：“数码管显示功能控制”按键，按下切换【0-打开数码管显示功能；1-关闭数码管显示功能】
			if(++Seg_Enable_Mode==2)Seg_Enable_Mode=0;
		break;
	}	
}
/*数码管显示区域*/
void Seg_Proc()
{
	unsigned char i=0;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	if(DAC_Voltage_OutPut_Mode==0)//0-DAC输出电压跟随RB2电压变化
	{
		AD_RB2_U=AD_Read(0x43);
		Voltage=AD_RB2_U/51.0;
	}
	else//1-输出固定电压2.0V，不随RB2电压变化
	{
		Voltage=2.0;
		DA_Write((Voltage/5)*255);
	}
	
	if(Seg_Enable_Mode==0)//0-打开数码管显示功能
	{
		switch(Seg_Show_Mode)
		{
			case 0:
				Seg_Point[5]=0;//清除小数点
				Seg_Buf[0]=11;//F
				Seg_Buf[1]=10;//熄灭
				Seg_Buf[2]=(Frequency/100000%10)?Frequency/100000%10:10;
				Seg_Buf[3]=(Frequency/10000%10)?Frequency/10000%10:10;
				Seg_Buf[4]=(Frequency/1000%10)?Frequency/1000%10:10;
				Seg_Buf[5]=(Frequency/100%10)?Frequency/100%10:10;
				Seg_Buf[6]=(Frequency/10%10)?Frequency/10%10:10;
				Seg_Buf[7]=(Frequency/1%10)?Frequency/1%10:10;
				for(i=2;i<8;i++)
				{
					
				}
			break;
			case 1:
				Seg_Buf[0]=12;//U
				Seg_Buf[1]=10;//熄灭
				Seg_Buf[2]=10;//熄灭
				Seg_Buf[3]=10;//熄灭
				Seg_Buf[4]=10;//熄灭
				Seg_Buf[5]=(unsigned int)(Voltage*100)/100%10;
				Seg_Point[5]=1;//小数点.
				Seg_Buf[6]=(unsigned int)(Voltage*100)/10%10;
				Seg_Buf[7]=(unsigned int)(Voltage*100)/1%10;
			break;
		}
	}
	else//1-关闭数码管显示功能
	{
		for(i=0;i<8;i++)
		{
			Seg_Buf[i]=10;//熄灭
			Seg_Point[i]=0;
		}
	}
}
/*Led显示区域*/
void Led_Proc()
{
	unsigned char i=0;
	if(Led_Enable_Mode==0)//0-打开LED指示功能
	{
		if(Seg_Show_Mode==0)//0-频率.频率测量功能指示：L1熄灭，L2点亮 
		{
			Led_Buf[0]=0;
			Led_Buf[1]=1;
		}
		else//1-电压.电压测量功能指示：L1点亮，L2熄灭
		{
			Led_Buf[0]=1;
			Led_Buf[1]=0;
		}
		
		
		if(Voltage<1.5)
			Led_Buf[2]=0;
		else if(Voltage>=1.5&&Voltage<2.5)
			Led_Buf[2]=1;
		else if(Voltage>=2.5&&Voltage<3.5)
			Led_Buf[2]=0;
		else if(Voltage>=3.5)
			Led_Buf[2]=1;
		
		
		if(Frequency<1000)
			Led_Buf[3]=0;
		else if(Frequency>=1000&&Frequency<5000)
			Led_Buf[3]=1;
		else if(Frequency>=5000&&Frequency<10000)
			Led_Buf[3]=0;
		else if(Frequency>=10000)
			Led_Buf[3]=1;
		

		if(DAC_Voltage_OutPut_Mode==0)//0-VRB2
			Led_Buf[4]=1;
		else//1-固定2.0V
			Led_Buf[4]=0;
		
	}
	else//1-关闭LED指示功能
	{
		for(i=0;i<8;i++)
		{
			Led_Buf[i]=0;
		}
	}
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
	if(++Seg_Slow==200)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	if(++Time_1000ms==1000)
	{
		Time_1000ms=0;
		Frequency=TH0<<8|TL0;
		TL0 = 0x00;				//设置定时初始值
		TH0 = 0x00;				//设置定时初始值
	}
	
}
/*定时器0区域*/
void Timer0_Init(void)		//0毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer1_Init();
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
