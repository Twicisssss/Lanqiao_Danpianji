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
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8]={0,0,0,0,0,0,0,0};
unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};

unsigned char Seg_Show_Mode=0;//【0-温度显示】【1-参数设置】【2-DAC输出】
unsigned char DAC_OutPut_Mode=0;//DAC输出电压模式【0-与温度相关】【1-按图7函数】

float Temperature;//读取温度
float Temperature_Parameter=25.0;//温度参数
float Temperature_Parameter_Set=25.0;//温度参数设置值
float Voltage_OutPut;//DAC输出电压值
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
		case 4://S4：“界面切换”按键，按下切换【0-温度显示】【1-参数设置】【2-DAC输出】界面
			if(++Seg_Show_Mode==3)Seg_Show_Mode=0;
			if(Seg_Show_Mode==1)//进入【1-参数设置】
			{
				Temperature_Parameter_Set=Temperature_Parameter;
			}
			else if(Seg_Show_Mode==2)//离开【1-参数设置】
			{
				Temperature_Parameter=Temperature_Parameter_Set;
			}
		break;
		case 5://S5：“模式”切换按键。按下切换DAC输出电压模式【0-与温度相关】【1-按图7函数】
			if(++DAC_OutPut_Mode==2)DAC_OutPut_Mode=0;
			/*
			模式1：【0-与温度相关】实时温度<温度参数时DAC输出0V，否则(>=)DAC 输出5V。 
			模式2：【1-按图7函数】DAC按照图7给出的关系输出电压。 
			*/
		break;
		case 8://S8：“减”按键，在【1-参数设置】界面按下，温度参数减1。
			if(Seg_Show_Mode==1)
			{
				Temperature_Parameter_Set--;
			}
		break;
		case 9://S9：“加”按键，在【1-参数设置】界面按下，温度参数加1。
			if(Seg_Show_Mode==1)
			{
				Temperature_Parameter_Set++;
			}
		break;
	}
}
/*Seg显示区域*/
void Seg_Proc()
{
	unsigned char i;
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	Temperature=Temperature_Read();

	if(DAC_OutPut_Mode==0)//【0-与温度相关】
	{
		if(Temperature<Temperature_Parameter)
		{
			Voltage_OutPut=0;
		}
		else
		{
			Voltage_OutPut=5;
		}
	}
	else if(DAC_OutPut_Mode==1)//【1-按图7函数】
	{
		if(Temperature<=20)
		{
			Voltage_OutPut=1;
		}
		else if(Temperature>20 && Temperature<40)
		{
			Voltage_OutPut=(Temperature*0.15)-2;
		}
		else if(Temperature>=40)
		{
			Voltage_OutPut=4;
		}
	}
	DA_Write((unsigned char)(Voltage_OutPut*51.0));

	
	switch(Seg_Show_Mode)
	{
		case 0:/*【0-温度显示】*/
			for(i=1;i<=3;i++)
				{Seg_Buf[i]=10;}//熄灭
			Seg_Buf[0]=11;//C
			Seg_Buf[4]=(unsigned int)Temperature/10%10;
			Seg_Buf[5]=(unsigned int)Temperature/1%10;
			Seg_Point[5]=1;//小数点
			Seg_Buf[6]=(unsigned int)(Temperature*10)%10;
			Seg_Buf[7]=(unsigned int)(Temperature*100)%10;
		break;
		case 1:/*【1-参数设置】*/
			Seg_Point[5]=0;//清除小数点
			for(i=1;i<=5;i++)
				{Seg_Buf[i]=10;}//熄灭
			Seg_Buf[0]=12;//P
			Seg_Buf[6]=(unsigned int)(Temperature_Parameter_Set/10)%10;
			Seg_Buf[7]=(unsigned int)(Temperature_Parameter_Set/1)%10;
		break;
		case 2:/*【2-DAC输出】*/
			for(i=1;i<=4;i++)
				{Seg_Buf[i]=10;}//熄灭
			Seg_Buf[0]=13;//A
			Seg_Buf[5]=(unsigned int)(Voltage_OutPut)/1%10;
			Seg_Point[5]=1;//小数点
			Seg_Buf[6]=(unsigned int)(Voltage_OutPut*10)%10;
			Seg_Buf[7]=(unsigned int)(Voltage_OutPut*100)%10;
		break;
	}
}
/*Led显示区域*/
void Led_Proc()
{
	Led_Buf[0]=(DAC_OutPut_Mode==0)?1:0;//1) 当前处于模式1状态，指示灯L1点亮，否则熄灭。 
	Led_Buf[1]=(Seg_Show_Mode==0)?1:0;	//2) 当前处于温度显示界面，指示灯L2点亮，否则熄灭。 
	Led_Buf[2]=(Seg_Show_Mode==1)?1:0;	//3) 当前处于参数设置界面，指示灯L3点亮，否则熄灭。 
	Led_Buf[3]=(Seg_Show_Mode==2)?1:0;	//4) 当前处于DAC输出界面，指示灯L4点亮，否则熄灭。
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
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	
	
}
/*初始化区域*/
void Init_Proc()
{
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
