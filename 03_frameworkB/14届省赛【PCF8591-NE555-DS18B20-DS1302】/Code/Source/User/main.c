/*==========头文件引用区域==========*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <ds1302.H>
#include <onewire.H>
#include <iic.H>
/*==========变量声明区域==========*/
idata unsigned long int sys_tick;
idata unsigned char seg_pos;
idata unsigned char seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char key_val,key_old,key_down,key_up;

idata unsigned char seg_show_mode=0;//【0-时间】【1-回显】【2-参数】【3-温湿度】
idata unsigned char echo_index=0;//【0-温度】【1-湿度】【2-时间】

idata unsigned int ne555_frequency;
idata unsigned int ne555_cycle_1s;

idata unsigned char rtc[3]={23,59,55};
idata float temperature;
idata float temperature_trigger;
idata float temperature_trigger_old;
idata float temperature_max;
idata float temperature_average;
idata float humidity;
idata float humidity_trigger;
idata float humidity_trigger_old;
idata float humidity_max;
idata float humidity_average;
idata unsigned char trigger_num=0;
idata unsigned char trigger_time[3]={23,59,55};

idata unsigned char temperature_parameter=30;

idata float ad_light_voltage;
idata bit day_night_state;	//【0-亮状态】【1-暗状态】
idata bit day_night_state_old;//【0-亮状态】【1-暗状态】
idata bit trigger_flag;
idata unsigned int time_3s;

idata unsigned int time_2s;
idata bit key9_press_flag;

idata unsigned char time_100ms;
idata bit time_100ms_flag;
/*==========数据获取区域==========*/
void adda_task(void)
{
	if(trigger_flag==0)
	{
		ad_light_voltage=ad_read(0x01)/51.0;
		
		if(ad_light_voltage<=1.0)
			day_night_state=1;
		else
			day_night_state=0;
		
		if(day_night_state_old==0 && day_night_state==1)
		{
			temperature_trigger_old=temperature_trigger;
			humidity_trigger_old=humidity_trigger;
			
			temperature_trigger=temperature;
			humidity_trigger=humidity;
			if(humidity!=0)
			{
				if(temperature_max<temperature_trigger)
					temperature_max=temperature_trigger;
				temperature_average=(temperature_average*trigger_num+temperature_trigger)/(trigger_num+1);
				
				if(humidity_max<humidity_trigger)
					humidity_max=humidity_trigger;
				humidity_average=(humidity_average*trigger_num+humidity_trigger)/(trigger_num+1);
				
				trigger_time[0]=rtc[0];
				trigger_time[1]=rtc[1];
				trigger_time[2]=rtc[2];
				trigger_num++;
			}
			trigger_flag=1;
		}
		else
			trigger_flag=0;
		
		day_night_state_old=day_night_state;
	}
}
void rtc_task(void)
{
	rtc_read(rtc);
}
void temperature_task(void)
{
	temperature=temperature_read();
}
/*==========按键控制区域==========*/
void key_task(void)
{
	key_val=key_read();
	key_down=key_val&(key_old^key_val);
	key_up=~key_val&(key_old^key_val);
	key_old=key_val;
	
	if(trigger_flag==0)
	{
		if(seg_show_mode==1 && echo_index==2)
		{
			if(key_down==9)
			{
				key9_press_flag=1;
			}
			if(key_up==9)
			{
				if(time_2s>=2000)
				{
					temperature_trigger=temperature_trigger_old=temperature_max=temperature_average=0;
					humidity_trigger=humidity_trigger_old=humidity_max=humidity_average=0;
					trigger_num=0;
					trigger_time[0]=trigger_time[1]=trigger_time[2]=0;
				}
				time_2s=0;
				key9_press_flag=0;
			}
		}
	
		switch(key_down)
		{
			case 4:
				seg_show_mode=(++seg_show_mode)%3;
				echo_index=0;
			break;
			case 5:
				if(seg_show_mode==1)
					echo_index=(++echo_index)%3;
			break;
			case 8://+
				if(seg_show_mode==2)
				{
					temperature_parameter++;
					if(temperature_parameter>=100)
						temperature_parameter=99;
				}
			break;
			case 9://-
				if(seg_show_mode==2)
				{
					temperature_parameter--;
					if(temperature_parameter>=250)
						temperature_parameter=0;
				}
			break;
		}
	}
}
/*==========数码管控制区域==========*/
void seg_task(void)
{
	if(trigger_flag==0)
	{
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
				if(echo_index==0)
				{
					seg_buf[0]=12;
					seg_buf[1]=10;
					seg_buf[2]=!(trigger_num==0)?((unsigned char)temperature_max/10):10;
					seg_buf[3]=!(trigger_num==0)?((unsigned char)temperature_max%10):10;
					seg_buf[4]=!(trigger_num==0)?11:10;
					seg_buf[5]=!(trigger_num==0)?((unsigned char)temperature_average/10):10;
					seg_buf[6]=!(trigger_num==0)?((unsigned char)temperature_average%10+','):10;
					seg_buf[7]=!(trigger_num==0)?((unsigned int)(temperature_average*10)%10):10;
				}
				else if(echo_index==1)
				{
					seg_buf[0]=13;
					seg_buf[1]=10;
					seg_buf[2]=!(trigger_num==0)?((unsigned char)humidity_max/10):10;
					seg_buf[3]=!(trigger_num==0)?((unsigned char)humidity_max%10):10;
					seg_buf[4]=!(trigger_num==0)?11:10;
					seg_buf[5]=!(trigger_num==0)?((unsigned char)humidity_average/10):10;
					seg_buf[6]=!(trigger_num==0)?((unsigned char)humidity_average%10+','):10;
					seg_buf[7]=!(trigger_num==0)?((unsigned int)(humidity_average*10)%10):10;
//					seg_buf[5]=(unsigned char)ad_light_voltage/10;
//					seg_buf[6]=(unsigned char)ad_light_voltage%10+',';
//					seg_buf[7]=(unsigned int)(ad_light_voltage*10)%10;
				}
				else if(echo_index==2)
				{
					seg_buf[0]=14;
					seg_buf[1]=trigger_num/10;
					seg_buf[2]=trigger_num%10;
					seg_buf[3]=!(trigger_num==0)?(trigger_time[0]/10):10;
					seg_buf[4]=!(trigger_num==0)?(trigger_time[0]%10):10;
					seg_buf[5]=!(trigger_num==0)?11:10;
					seg_buf[6]=!(trigger_num==0)?(trigger_time[1]/10):10;
					seg_buf[7]=!(trigger_num==0)?(trigger_time[1]%10):10;
				}
			break;
			case 2:
				seg_buf[0]=15;
				seg_buf[1]=seg_buf[2]=seg_buf[3]=seg_buf[4]=seg_buf[5]=10;
				seg_buf[6]=temperature_parameter/10;
				seg_buf[7]=temperature_parameter%10;
			break;
		}
	}
	else
	{
		seg_buf[0]=16;
		seg_buf[1]=10;
		seg_buf[2]=10;
		seg_buf[3]=(unsigned char)temperature_trigger/10;
		seg_buf[4]=(unsigned char)temperature_trigger%10;
		seg_buf[5]=11;
		seg_buf[6]=!(humidity_trigger==0)?((unsigned char)humidity_trigger/10):17;
		seg_buf[7]=!(humidity_trigger==0)?((unsigned char)humidity_trigger%10):17;
	}
}
/*==========LED控制区域==========*/
void led_task(void)
{
	led_buf[0]=(trigger_flag==0 && seg_show_mode==0);
	led_buf[1]=(trigger_flag==0 && seg_show_mode==1);
	led_buf[2]=(trigger_flag==1);
	
	
//	led_buf[3]=(temperature_trigger>temperature_parameter)?time_100ms_flag:0;
//	led_buf[4]=(trigger_num>=1 && humidity_trigger==0);
	led_buf[3]=(temperature>temperature_parameter)?time_100ms_flag:0;
	led_buf[4]=(trigger_num>=1 && humidity==0);
	led_buf[5]=(trigger_num>=2 && temperature_trigger>temperature_trigger_old && humidity_trigger>humidity_trigger_old);
	
	led_disp(led_buf);
}
/*==========NE555定时器0区域==========*/
void NE555_Timer0_Init(void)		//1毫秒@12.000MHz
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
	if(++ne555_cycle_1s==100)
	{
		ne555_cycle_1s=0;
		ne555_frequency=TH0<<8|TL0;
		TH0=TL0=0;
		ne555_frequency=ne555_frequency*10;
		if(ne555_frequency>=200 && ne555_frequency<=2000)
			humidity=(8.0/180.0)*(ne555_frequency-200.0)+10.0;
		else
			humidity=0;
	}
	
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	
	if(trigger_flag==1)
	{
		if(++time_3s==3000)
		{
			trigger_flag=0;
			time_3s=0;
		}
	}
	
	if(key9_press_flag==1)
	{
		if(++time_2s>=2000)
		{
			time_2s=2000;
		}
	}
	else
		time_2s=0;
	
	
	if(temperature_trigger>temperature_parameter)
	{
		if(++time_100ms==100)
		{
			time_100ms=0;
			time_100ms_flag^=1;
		}
	}
	else
		time_100ms_flag=time_100ms=0;
}
/*==========调度器区域==========*/
typedef struct{
	void (*taskfunction)(void);
	unsigned long int rate_time;
	unsigned long int last_time;
}TaskMessage;
idata TaskMessage TaskSchedule[]={
	{key_task,20,0},
	{led_task,1,0},
	{seg_task,200,0},
	{rtc_task,300,0},
	{temperature_task,250,0},
	{adda_task,160,0},
};
idata unsigned char task_num;
void task_init(void)
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
			TaskSchedule[i].taskfunction();
		}
	}
}
/*==========初始化区域==========*/
void init_task(void)
{
	sys_init();
	task_init();
	rtc_write(rtc);
	while(temperature_read()==85)
		;
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
