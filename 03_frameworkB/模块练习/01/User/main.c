/*==========头文件引用区域==========*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <ds1302.H>
#include <onewire.H>
#include <iic.H>
#include <ultrasound.H>
#include <uart.H>
/*==========变量声明区域==========*/
idata unsigned long int sys_tick;
idata unsigned char key_val,key_old,key_down,key_up;
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char seg_pos,seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char seg_show_mode=0;

//数据
idata unsigned char rtc[3]={23,59,55};
idata unsigned int ne555_frequency;
idata unsigned int ne555_cycle_1s;
idata unsigned int temperature_10x;
idata unsigned char ad_light_v_10x;
idata unsigned char ad_RB2_v_10x;
idata unsigned char da_voltage_10x;
idata unsigned char us_distance;

//串口
pdata unsigned char uart_buf[10]={0};
idata unsigned char uart_rx_index;
idata unsigned int uart_rx_tick;
idata unsigned char uart_rx_flag;

//eeprom
idata unsigned char eeprom_save_data;
pdata unsigned char eeprom_pw=23;
/*==========数据获取区域==========*/
void rtc_task(void)
{
	rtc_read(rtc);
}
void temperature_task(void)
{
	temperature_10x=temperature_read()*10;
}
void adda_task(void)
{	
//	ad_light_v_10x=(ad_read(0x01)/51.0)*10;
	/*同时读取两个电压值时，需要调换地址*/
	ad_light_v_10x=(ad_read(0x03)/51.0)*10;
	ad_RB2_v_10x=(ad_read(0x01)/51.0)*10;
	
	da_voltage_10x=21;//输出2.1V电压
	da_write(da_voltage_10x*5.10);
}
void ulrtasound_task(void)
{
	us_distance=us_distance_get();
}
/*==========按键控制区域==========*/
void key_task(void)
{
	key_val=key_read();
	key_down=key_val&(key_old^key_val);
	key_up=~key_val&(key_old^key_val);
	key_old=key_val;
	
	switch(key_down)
	{
		case 4:
			seg_show_mode=(++seg_show_mode)%4;
		break;
		case 5:
			
			eeprom_write(&eeprom_save_data,0,1);
			eeprom_write(&eeprom_pw,8,1);
		break;
		case 8:
			eeprom_save_data++;
		break;
	}
}
/*==========数码管控制区域==========*/
void seg_task(void)
{
	unsigned char j;
	switch(seg_show_mode)
	{
		case 0:
			seg_buf[0]=rtc[0]/10;
			seg_buf[1]=rtc[0]%10;
			seg_buf[2]=11;
			seg_buf[3]=rtc[1]/10;
			seg_buf[4]=rtc[1]%10;
			seg_buf[5]=11;
			seg_buf[6]=rtc[2]/10;
			seg_buf[7]=rtc[2]%10;
		break;
		case 1:
			seg_buf[0]=12;
			seg_buf[1]=12;
			seg_buf[2]=ne555_frequency/100000%10;
			seg_buf[3]=ne555_frequency/10000%10;
			seg_buf[4]=ne555_frequency/1000%10;
			seg_buf[5]=ne555_frequency/100%10;
			seg_buf[6]=ne555_frequency/10%10;
			seg_buf[7]=ne555_frequency/1%10;
			for(j=2;j<7;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]>=10)
					seg_buf[j]=10;
			}
		break;
		case 2:
			seg_buf[0]=13;
			seg_buf[1]=temperature_10x/100%10;
			seg_buf[2]=temperature_10x/10%10+',';
			seg_buf[3]=temperature_10x/1%10;
			seg_buf[4]=15;
			seg_buf[5]=us_distance/100%10;
			seg_buf[6]=us_distance/10%10;
			seg_buf[7]=us_distance/1%10;
		break;
		case 3:
			seg_buf[0]=14;
			seg_buf[1]=11;
			seg_buf[2]=ad_light_v_10x/10%10+',';
			seg_buf[3]=ad_light_v_10x/1%10;
			seg_buf[4]=14;
			seg_buf[5]=11;
			seg_buf[6]=ad_RB2_v_10x/10%10+',';
			seg_buf[7]=ad_RB2_v_10x/1%10;
		break;
	}
}
/*==========LED控制区域==========*/
void led_task(void)
{
	led_buf[0]=1;
	
	led_disp(led_buf);
}
/*==========NE555定时器0区域==========*/
void NE555_Timer0_Init(void)//@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
/*==========定时器1区域==========*/
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
	sys_tick++;
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>=20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	if(++ne555_cycle_1s==100)
	{
		ne555_cycle_1s=0;
		ne555_frequency=TH0<<8|TL0;
		TH0=TL0=0;
		ne555_frequency=ne555_frequency*10;
	}
	
	if(uart_rx_flag==1)
		uart_rx_tick++;
}
/*==========串口控制区域==========*/
void uart_task(void)
{
	unsigned int num;
	unsigned int result,chars_read;
	if(uart_rx_index==0)return;
	if(uart_rx_tick>10)
	{
		uart_rx_flag=0;
		uart_rx_tick=0;
		if(strncmp(uart_buf,"show=",5)==0)
			result=sscanf(uart_buf,"show=%u%n",&num,&chars_read);
		if(result==1)
		{
			seg_show_mode=num;
			printf("Success!show%u\n",num);
			printf("chars_read=%u\n",chars_read);
			printf("eeprom_data=%bu\n",eeprom_save_data);
		}
		
		memset(uart_buf,0,uart_rx_index);
		uart_rx_index=0;
	}
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		uart_rx_flag=1;
		uart_rx_tick=0;
		uart_buf[uart_rx_index++]=SBUF;
		
		RI = 0;			//清除串口1接收中断请求位
		if(uart_rx_index>10)
		{
			uart_rx_index=0;
			memset(uart_buf,0,10);
		}
	}
}
/*==========调度器区域==========*/
typedef struct{
	void (*task_function)(void);
	unsigned long int rate_time;
	unsigned long int last_time;
}TaskMessage;
idata TaskMessage TaskSchedule[]={
	{key_task,20,0},
	{led_task,1,0},
	{seg_task,200,0},
	{rtc_task,300,0},
	{temperature_task,180,0},
	{ulrtasound_task,100,0},
	{adda_task,160,0},
	{uart_task,260,0},
};
idata task_num;
void schedule_init(void)
{
	task_num=sizeof(TaskSchedule)/sizeof(TaskMessage);
}
void schedule_run(void)
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;
		if(now_time>=TaskSchedule[i].rate_time+TaskSchedule[i].last_time)
		{
			TaskSchedule[i].last_time=now_time;
			TaskSchedule[i].task_function();
		}
	}
}
/*==========初始化区域==========*/
void init_task(void)
{
	unsigned char eeprom_check;
	eeprom_read(&eeprom_check,8,1);
	if(eeprom_check==23)
		eeprom_read(&eeprom_save_data,0,1);
	sys_init();
	schedule_init();
	rtc_set(rtc);
	Uart1_Init();
	NE555_Timer0_Init();
	Timer1_Init();
}
/*==========主函数区域==========*/
void main(void)
{
	init_task();
	while(1)
	{
		schedule_run();
	}
}


