/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Led.H>
#include <Seg.H>
#include <Key.H>
#include <onewire.H>
#include <iic.H>
#include <Uart.H>
/*变量声明区域*/
idata unsigned char Key_Slow,Key_Val,Key_Old,Key_Down,Key_Up;
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10},Seg_Point[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0,Relay_Buf=0,Led_Buf[8]={0,0,0,0,0,0,0,0};
pdata unsigned char Uart_Buf[5]={0};
idata unsigned char Uart_Rx_Index;
idata unsigned char Uart_Recv_Tick;
idata unsigned char Uart_Rx_Flag;


idata unsigned char Seg_Show_Mode=0;//【0-温度】【1-电压】
idata bit Uart_Ctrl_Seg_Mode=1;//串口切换界面功能【0-失效】【1-生效】

idata float Temperature;
idata float ADC_RB2_Voltage;

idata unsigned char Time_100ms;
idata bit Time_100ms_Flag;

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
			Uart_Ctrl_Seg_Mode=0;
			Time_100ms=0;
		break;
		case 5:
			if(Uart_Ctrl_Seg_Mode==0)
				Uart_Ctrl_Seg_Mode=1;
		break;
		case 12:
			if(Seg_Show_Mode==0)
				printf("TEMP:%.1f℃\r\n",Temperature);
			else
				printf("Voltage:%.2fV\r\n",ADC_RB2_Voltage);
		break;
	}
}
/*数码管控制区域*/
void Seg_Proc()
{	
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	
	Temperature=Temperature_Read();//onewire:ds18b20
	ADC_RB2_Voltage=AD_Read(0x03)/51.0;//iic:PCF8591
	
	
	Seg_Buf[0]=11;
	Seg_Buf[1]=Seg_Show_Mode+1;
	Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Point[5]=0;
			Seg_Buf[5]=(unsigned char)(Temperature)/10%10;
			Seg_Buf[6]=(unsigned char)(Temperature)/1%10;
			Seg_Point[6]=1;
			Seg_Buf[7]=(unsigned int)(Temperature*10)%10;
			
		break;
		case 1:
			Seg_Point[6]=0;
			Seg_Buf[5]=(unsigned char)(ADC_RB2_Voltage)%10;
			Seg_Point[5]=1;
			Seg_Buf[6]=(unsigned int)(ADC_RB2_Voltage*10)%10;
			Seg_Buf[7]=(unsigned int)(ADC_RB2_Voltage*100)%10;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
	Led_Buf[2]=Uart_Ctrl_Seg_Mode==0?Time_100ms_Flag:0;
	
	Relay_Buf=(Temperature>=28);
	Beeper_Buf=(ADC_RB2_Voltage>3.6);
}
/*串口控制区域*/
void Uart_Proc()
{
	if(Uart_Rx_Index==0)
		return;
	if(Uart_Recv_Tick>=10)
	{
		Uart_Recv_Tick=0;
//		Uart_Rx_Flag = 0;
		
		if(Uart_Ctrl_Seg_Mode==1)
		{
			if(Uart_Buf[0]=='A')
				Seg_Show_Mode=0;
			else if(Uart_Buf[0]=='B')
				Seg_Show_Mode=1;
		}
		
		
		memset(Uart_Buf,0,Uart_Rx_Index);// 清空接收数据
		Uart_Rx_Index = 0;
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Beeper(Beeper_Buf);
	Relay(Relay_Buf);
	
	if(Uart_Ctrl_Seg_Mode==0)
	{
		if(++Time_100ms==100)
		{
			Time_100ms=0;
			Time_100ms_Flag^=1;
		}
	}
	
	if(Uart_Rx_Flag==1)
	{
		Uart_Recv_Tick++;
	}
}
/*串口中断区域*/
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		Uart_Rx_Flag=1;
		Uart_Recv_Tick=0;
		Uart_Buf[Uart_Rx_Index++]=SBUF;
		RI = 0;			//清除串口1接收中断请求位
		if(Uart_Rx_Index>10)
		{
			Uart_Rx_Index=0;
			memset(Uart_Buf,0,10);
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	while(Temperature_Read()==85);//onewire:ds18b20
	Sys_Init();
	Uart1_Init();
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
		Uart_Proc();
	}
}
