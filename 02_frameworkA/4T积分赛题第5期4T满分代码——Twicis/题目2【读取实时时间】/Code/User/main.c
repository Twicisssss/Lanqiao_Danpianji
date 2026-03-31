/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <Init.H>
#include <Seg.H>
#include <Uart.H>
#include <ds1302.H>
#include <Beeper.H>
/*变量声明区域*/
idata unsigned char Seg_Slow,Seg_Pos,Seg_Buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char Beeper_Buf=0;
idata unsigned char Rtc[3]={23,59,59};

idata unsigned int Time_2000ms;

idata unsigned char Uart_Buf[10]={0};
idata unsigned char Uart_Index;
idata unsigned char Uart_Rx_Flag;
idata unsigned char Uart_Tick;
idata int result,chars_read;
idata unsigned char hour,minute,second;

idata unsigned char Rtc_Alarm[3];
/*数码管控制区域*/
void Seg_Proc()
{
	if(Seg_Slow)return;
	Seg_Slow=1;
	
	RTC_Read(Rtc);
	
	Seg_Buf[0]=Rtc[0]/10%10;
	Seg_Buf[1]=Rtc[0]%10;
	Seg_Buf[2]=11;
	Seg_Buf[3]=Rtc[1]/10%10;
	Seg_Buf[4]=Rtc[1]%10;
	Seg_Buf[5]=11;
	Seg_Buf[6]=Rtc[2]/10%10;
	Seg_Buf[7]=Rtc[2]%10;
}
/*蜂鸣器控制区域*/
void Beeper_Proc()
{
	if(Rtc[0]==Rtc_Alarm[0] && Rtc[1]==Rtc_Alarm[1] && Rtc[2]==Rtc_Alarm[2])
	{
		Beeper_Buf=1;
	}
}
/*串口控制区域*/
void Uart_Proc()
{
	if(Uart_Index==0)return;
	if(Uart_Tick>=10)
	{
		Uart_Tick=0;
		Uart_Rx_Flag=0;
		result=sscanf((char*)Uart_Buf,"%bu：%bu：%bu%n",&hour,&minute,&second,&chars_read);
		if(result==3 && chars_read==strlen((char*)Uart_Buf))
		{
			Rtc_Alarm[0]=hour;
			Rtc_Alarm[1]=minute;
			Rtc_Alarm[2]=second;
//			printf("chars_read=%u\r\nstrlen=%u\r\n",chars_read,strlen((char*)Uart_Buf));
//			printf("hour=%bu\r\nminute=%bu\r\nsecond=%bu",hour,minute,second);
		}
		else
		{
			printf("error");
		}
		
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
	if(++Seg_Slow==90)Seg_Slow=0;
	if(++Seg_Pos==8)Seg_Pos=0;
	if(Seg_Buf[Seg_Pos]>20)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos]-',',1);
	else
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],0);
	Beeper(Beeper_Buf);
	
	
	
	if(Uart_Rx_Flag==1)Uart_Tick++;
	
	if(Beeper_Buf==1)
	{
		if(++Time_2000ms==2000)
		{
			Time_2000ms=0;
			Beeper_Buf=0;
		}
	}
}
/*初始化区域*/
void Init_Proc()
{
	RTC_Set(Rtc);
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
		Seg_Proc();
		Uart_Proc();
		Beeper_Proc();
	}
}
