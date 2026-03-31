/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <ds1302.H>
#include <iic.H>
#include <onewire.H>
#include <ultrasound.H>
#include <usart.H>
/*变量声明区域*/
idata unsigned long int sys_tick=0;
idata unsigned char seg_pos,seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char key_val,key_old,key_down,key_up;
idata unsigned char seg_show_mode=0;

idata float ad_light_100x,ad_rb2_100x;
idata unsigned char eeprom_write_data,eeprom_read_data;
idata float temperature;
idata unsigned char rtc[3]={23,59,55};
idata unsigned char us_distance;
idata unsigned int ne555_frequency;
idata unsigned int time_1s_freq;

idata unsigned char uart_buf[10];
idata unsigned char uart_rx_index;
idata unsigned char uart_tick;
idata bit uart_rx_flag;

idata unsigned char pwm_compare=8;
idata unsigned char pwm_count=0;

/*数据获取区域*/
void adda_task()
{
	ad_light_100x=ad_read(0x41)/51.0;
	ad_rb2_100x=ad_read(0x43)/51.0;
	da_write(2.3*51);
}
void temperature_task()
{
	temperature=temperature_read();
}
void rtc_task()
{
	rtc_read(rtc);
}
void us_distance_task()
{
	us_distance=us_distance_get();
}
/*按键控制区域*/
void key_task()
{
	key_val=key_read();
	key_down=key_val^(key_val&key_old);
	key_up=~key_val^(key_val&key_old);
	key_old=key_val;
	
	if(key_down==4)
	{
		eeprom_write_data=5;
		eeprom_write(&eeprom_write_data,0,1);
		seg_show_mode=(++seg_show_mode)%4;
	}
	else if(key_down==5)
	{
		pwm_compare=(++pwm_compare)%10;
	}

}
/*数码管控制区域*/
void seg_task()
{
	switch(seg_show_mode)
	{
		case 0:
			seg_buf[0]=(unsigned char)(ad_light_100x*1)%10+',';
			seg_buf[1]=(unsigned char)(ad_light_100x*10)%10;
			seg_buf[2]=(unsigned int)(ad_light_100x*100)%10;
			seg_buf[3]=10;
			seg_buf[4]=(unsigned char)(ad_rb2_100x*1)%10+',';
			seg_buf[5]=(unsigned char)(ad_rb2_100x*10)%10;
			seg_buf[6]=(unsigned int)(ad_rb2_100x*100)%10;
			seg_buf[7]=10;
		break;
		case 1:
			seg_buf[0]=(unsigned char)temperature/10%10;
			seg_buf[1]=(unsigned char)temperature/1%10+',';
			seg_buf[2]=(unsigned char)(temperature*10)%10;
			seg_buf[3]=(unsigned int)(temperature*100)%10;
			seg_buf[4]=10;
		    seg_buf[5]=us_distance/100%10;
		    seg_buf[6]=us_distance/10%10;
			seg_buf[7]=us_distance/1%10;
		break;
		case 2:
			seg_buf[0]=rtc[0]/10;
			seg_buf[1]=rtc[0]%10;
			seg_buf[2]=10;
			seg_buf[3]=rtc[1]/10;
			seg_buf[4]=rtc[1]%10;
		    seg_buf[5]=10;
		    seg_buf[6]=rtc[2]/10;
			seg_buf[7]=rtc[2]%10;
		break;
		case 3:
			seg_buf[0]=eeprom_read_data;
			seg_buf[1]=10;
			seg_buf[2]=ne555_frequency/100000%10;
			seg_buf[3]=ne555_frequency/10000%10;
			seg_buf[4]=ne555_frequency/1000%10;
		    seg_buf[5]=ne555_frequency/100%10;
		    seg_buf[6]=ne555_frequency/10%10;
			seg_buf[7]=ne555_frequency/1%10;
		break;
	}
	
}
/*led控制区域*/
void led_task()
{
	led_buf[0]=1;
	
	
	relay(led_buf[7]);beep(led_buf[7]);
}
/*串口控制区域*/
void uart_task()
{
	unsigned char x,y;
	if(uart_rx_index==0)return;
	if(uart_tick>=10)
	{
		uart_rx_flag=0;
		uart_tick=0;
		
		if(sscanf(uart_buf,"%bu,%bu",&x,&y)==2)
			printf("x=%bu,y=%bu",x,y);
		else
			printf("error");
		
		memset(uart_buf,0,uart_rx_index);
		uart_rx_index=0;
	}
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		uart_rx_flag=1;
		uart_tick=0;
		
		uart_buf[uart_rx_index++]=SBUF;
		RI = 0;			//清除串口1接收中断请求位
		if(uart_rx_index>10)
		{
			uart_rx_index=0;
			memset(uart_buf,0,10);
		}
	}
}
/*NE555定时器0区域*/
void NE555_Timer0_Init(void)
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
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
}
void Timer1_Isr(void) interrupt 3
{	
	sys_tick++;
	if(uart_rx_flag)
		uart_tick++;
	if(++time_1s_freq==100)
	{
		time_1s_freq=0;
		ne555_frequency=(TH0<<8)|TL0;
		TH0=TL0=0;
		ne555_frequency=ne555_frequency*10;
	}
	pwm_count=(++pwm_count)%10;
	if(pwm_count>pwm_compare)
		led_off();
	else
		led_disp(led_buf);
	
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
}
/*调度器区域*/
typedef struct{
	void (*pTaskFunc)(void);
	unsigned long int RateTime;
	unsigned long int LastTime;
}TaskT;
idata TaskT SchedulerTask[]={
	{key_task,10,0},
	{seg_task,50,0},
	{led_task,1,0},
	{adda_task,160,0},
	{temperature_task,200,0},
	{rtc_task,100,0},
	{us_distance_task,80,0},
	{uart_task,300,0},
};
idata unsigned char task_num;
void scheduler_init()
{
	task_num=sizeof(SchedulerTask)/sizeof(TaskT);
}
void scheduler_run()
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;//获取当前时间
		if(now_time>=SchedulerTask[i].RateTime+SchedulerTask[i].LastTime)
		{
			SchedulerTask[i].LastTime=now_time;
			SchedulerTask[i].pTaskFunc();
		}
	}
}
/*初始化区域*/
void init_task()
{
	sys_init();
	NE555_Timer0_Init();
	Timer1_Init();
	scheduler_init();
	eeprom_read(&eeprom_read_data,0,1);
	rtc_set(rtc);
	Uart1_Init();
	EA=1;
}
/*主函数区域*/
void main()
{
	init_task();
	while(1)
	{
		scheduler_run();
	}
}
