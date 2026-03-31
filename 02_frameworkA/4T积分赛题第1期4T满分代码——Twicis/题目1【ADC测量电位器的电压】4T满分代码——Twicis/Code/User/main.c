/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <iic.H>
#include <Led.H>
#include <Uart.H>

/*变量声明区域*/
pdata unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Seg_Slow,Seg_Pos;
idata unsigned char Led_Buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char Beeper_Buf=0;
idata unsigned char Relay_Buf=0;
idata unsigned char Uart_Buf[10]={0};
idata unsigned char Uart_Index;
idata unsigned char Uart_Rx_Flag;
idata unsigned char Uart_Tick;
idata char Uart_Char[6];
idata int chars_read;
idata int result;

idata unsigned char i;

idata unsigned char Seg_Show_Mode=2;//【0-一位小数】【0-两位小数】【0-整数】

idata float ADC_Rb2_Voltage;
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	ADC_Rb2_Voltage=AD_Read(0x03)/51.0;
	
	Seg_Buf[0]=11;
	Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
	switch(Seg_Show_Mode)
	{
		case 0:
			Seg_Buf[5]=10;
			Seg_Buf[6]=(unsigned char)(ADC_Rb2_Voltage)%10+',';
			Seg_Buf[7]=(unsigned char)(ADC_Rb2_Voltage*10)%10;
		break;
		case 1:
			Seg_Buf[5]=(unsigned char)(ADC_Rb2_Voltage)%10+',';
			Seg_Buf[6]=(unsigned char)(ADC_Rb2_Voltage*10)%10;
			Seg_Buf[7]=(unsigned int)(ADC_Rb2_Voltage*100)%10;
		break;
		case 2:
			Seg_Buf[5]=10;
			Seg_Buf[6]=10;
			Seg_Buf[7]=(unsigned char)(ADC_Rb2_Voltage)%10;
		break;
	}
}
/*Led控制区域*/
void Led_Proc()
{
	Relay_Buf=Beeper_Buf=0;

	Led_Buf[0]=Seg_Show_Mode==0?1:0;
	Led_Buf[1]=Seg_Show_Mode==1?1:0;
}
/*串口控制区域*/
void Uart_Proc()
{
	if(Uart_Index==0)return;
	if(Uart_Tick>=10)
	{
		if((Uart_Buf[0]=='a'||Uart_Buf[0]=='A') || (Uart_Buf[1]=='a'||Uart_Buf[1]=='A') || (Uart_Buf[2]=='a'||Uart_Buf[2]=='A') || (Uart_Buf[3]=='a'||Uart_Buf[3]=='A') || (Uart_Buf[4]=='a'||Uart_Buf[4]=='A'))
			Seg_Show_Mode=0;
//			printf("0000");
		else if((Uart_Buf[0]=='1'||Uart_Buf[0]=='#') || (Uart_Buf[1]=='1'||Uart_Buf[1]=='#') || (Uart_Buf[2]=='1'||Uart_Buf[2]=='#') || (Uart_Buf[3]=='1'||Uart_Buf[3]=='#') || (Uart_Buf[4]=='1'||Uart_Buf[4]=='#'))
			Seg_Show_Mode=1;
//			printf("11111");
		else
			Seg_Show_Mode=2;
//			printf("22222");
		
		Uart_Tick=0;
		memset(Uart_Buf,0,Uart_Index);
		Uart_Index=0;
	}
}
/*串口中断区域*/
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		Uart_Rx_Flag=1;
		Uart_Tick=0;
		Uart_Buf[Uart_Index++]=SBUF;
		RI = 0;			//清除串口1接收中断请求位
		if(Uart_Index>10)
		{
			Uart_Index=0;
			memset(Uart_Buf,0,10);
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
	if(++Seg_Slow==100)Seg_Slow=0;
	
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	Led_Disp(Seg_Pos,Led_Buf[Seg_Pos]);
	Relay(Relay_Buf);
	Beeper(Beeper_Buf);
	
	
	if(Uart_Rx_Flag==1)Uart_Tick++;
}
/*初始化区域*/
void Init_Proc()
{
	Sys_Init();
	Timer1_Init();
	Uart1_Init();
}
/*主函数区域*/
void main()
{
	Init_Proc();
	while(1)
	{
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}
