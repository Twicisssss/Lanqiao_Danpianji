/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <iic.H>
#include <ultrasound.H>
/*变量声明区域*/
idata unsigned long int sys_tick;
idata bit relay_buf;
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char seg_pos;
idata unsigned char seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char key_val,key_old,key_down,key_up;
idata unsigned char seg_show_mode=0;//【0-频率】【1-湿度】【2-测距】【3-参数】
idata unsigned char parameter_index=0;//【0-频率参数P1】【1-湿度参数P2】【2-距离参数P3】

idata unsigned int ne555_frequency;
idata unsigned int time_1s_f;
idata bit freq_Hz_KHz=0;

idata float ad_rb2_voltage;
idata unsigned char humidity_100x;
idata float da_voltage;

idata unsigned char us_distance;
idata bit dist_CM_M=0;

idata unsigned int parameter_frequency=9000;
idata unsigned char parameter_humidity=40;
idata unsigned char parameter_distance=60;

idata unsigned char pwm_count;
idata unsigned char pwm_cycle=10;//因为100us*10=1ms

idata unsigned char time_100ms;
idata bit time_100ms_flag;

idata bit relay_buf_old;
idata unsigned char relay_count;
idata unsigned char eeprom_check=6;

idata bit key7_press_flag;
idata unsigned int key7_press_1s;
/*数据获取区域*/
void adda_task(void)
{
	ad_rb2_voltage=ad_read(0x43)/51.0;
	
	humidity_100x=ad_rb2_voltage*20.0;
	if(humidity_100x==100)humidity_100x=99;
	
	if(humidity_100x<=parameter_humidity)
		da_voltage=1.0;
	else if(humidity_100x>=80)
		da_voltage=5.0;
	else if(humidity_100x>parameter_humidity && humidity_100x<80)
		da_voltage=((4.0/(80.0-parameter_humidity))*humidity_100x)+(5.0-(320.0/(80.0-parameter_humidity)));
	
	da_write(da_voltage*51.0);
}
void us_dist_task(void)
{
	us_distance=us_distance_get()+3;
}
/*按键控制区域*/
void key_task(void)
{
	key_val=key_read();
	key_down=key_val&(key_old^key_val);
	key_up=~key_val&(key_old^key_val);
	key_old=key_val;
	
	if(key_down==7 && seg_show_mode==1)//在湿度界面下，长按S7按键超过1秒后松开，清零继电器开关次数统计
	{
		key7_press_flag=1;
	}
	if(key_up==7)
	{
		if(key7_press_1s>=1000)
		{
			relay_count=0;
			eeprom_write(&relay_count,0,1);
			eeprom_write(&eeprom_check,8,1);
		}
		key7_press_flag=0;
		key7_press_1s=0;
	}
	
	if(key_down==4)
	{
		seg_show_mode=(++seg_show_mode)%4;
		parameter_index=0;
	}
	else if(key_down==5)
	{
		parameter_index=(++parameter_index)%3;
		time_100ms=time_100ms_flag=0;
	}
	else if(key_down==6)//++
	{
		if(seg_show_mode==2)//【2-测距】
			dist_CM_M^=1;
		else if(seg_show_mode==3)//【3-参数】
		{
			if(parameter_index==0)//【0-频率参数P1】
			{
				parameter_frequency+=500;
				if(parameter_frequency>12000)
					parameter_frequency=1000;
			}
			else if(parameter_index==1)//【1-湿度参数P2】
			{
				parameter_humidity+=10;
				if(parameter_humidity>60)
					parameter_humidity=10;
			}
			else if(parameter_index==2)//【2-距离参数P3】
			{
				parameter_distance+=10;
				if(parameter_distance>120)
					parameter_distance=10;
			}
		}
	}
	else if(key_down==7)//--
	{
		if(seg_show_mode==0)//【0-频率】
			freq_Hz_KHz^=1;
		else if(seg_show_mode==3)//【3-参数】
		{
			if(parameter_index==0)//【0-频率参数P1】
			{
				parameter_frequency-=500;
				if(parameter_frequency<1000)
					parameter_frequency=12000;
			}
			else if(parameter_index==1)//【1-湿度参数P2】
			{
				parameter_humidity-=10;
				if(parameter_humidity<10)
					parameter_humidity=60;
			}
			else if(parameter_index==2)//【2-距离参数P3】
			{
				parameter_distance-=10;
				if(parameter_distance<10)
					parameter_distance=120;
			}
		}
	}
}
/*数码管控制区域*/
void seg_task(void)
{
	unsigned char j;
	switch(seg_show_mode)
    {
    	case 0:
			seg_buf[0]=11;
			seg_buf[1]=10;
			seg_buf[2]=!freq_Hz_KHz?(ne555_frequency/100000%10):(ne555_frequency/10000000%10);
			seg_buf[3]=!freq_Hz_KHz?(ne555_frequency/10000%10):(ne555_frequency/1000000%10);
			seg_buf[4]=!freq_Hz_KHz?(ne555_frequency/1000%10):(ne555_frequency/100000%10);
			seg_buf[5]=!freq_Hz_KHz?(ne555_frequency/100%10):(ne555_frequency/10000%10);
			seg_buf[6]=!freq_Hz_KHz?(ne555_frequency/10%10):(ne555_frequency/1000%10+',');
		    seg_buf[7]=!freq_Hz_KHz?(ne555_frequency/1%10):(ne555_frequency/100%10);
			for(j=2;j<7;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]==10)
					seg_buf[j]=10;
			}
			break;
    	case 1:
			seg_buf[0]=12;
			seg_buf[1]=seg_buf[2]=seg_buf[3]=seg_buf[4]=seg_buf[5]=10;
			seg_buf[6]=humidity_100x/10;
		    seg_buf[7]=humidity_100x%10;
    	break;
    	case 2:
			seg_buf[0]=13;
			seg_buf[1]=seg_buf[2]=seg_buf[3]=seg_buf[4]=10;
			seg_buf[5]=!dist_CM_M?(us_distance/100%10):(us_distance/100%10+',');
			seg_buf[6]=us_distance/10%10;
			seg_buf[7]=us_distance/1%10;
			for(j=5;j<7;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]==10)
					seg_buf[j]=10;
			}
    	break;
    	case 3:
			seg_buf[0]=14;
			seg_buf[1]=parameter_index+1;
			seg_buf[2]=seg_buf[3]=seg_buf[4]=10;
			if(parameter_index==0)//【0-频率参数KHz】1.0KHz－12.0KHz
			{
				seg_buf[5]=parameter_frequency/10000%10;
				seg_buf[6]=parameter_frequency/1000%10+',';
				seg_buf[7]=parameter_frequency/100%10;
				for(j=5;j<7;j++)
				{
					if(seg_buf[j]==0 && seg_buf[j-1]==10)
						seg_buf[j]=10;
				}
			}
			else if(parameter_index==1)//【1-湿度参数】10%-60%
			{
				seg_buf[5]=10;
				seg_buf[6]=parameter_humidity/10;
				seg_buf[7]=parameter_humidity%10;
			}
			else if(parameter_index==2)//【2-距离参数】0.1M－1.2M
			{
				seg_buf[5]=10;
				seg_buf[6]=parameter_distance/100%10+',';
				seg_buf[7]=parameter_distance/10%10;
			}
    	break;
    }
}
/*LED控制区域*/
void led_task(void)
{
	relay_buf_old=relay_buf;
	relay_buf=us_distance>parameter_distance;
	if(relay_buf_old!=relay_buf)
	{
		relay_count++;
		eeprom_write(&relay_count,0,1);
		eeprom_write(&eeprom_check,8,1);
	}

	if(seg_show_mode!=3)
	{
		led_buf[0]=(seg_show_mode==0);
		led_buf[1]=(seg_show_mode==1);
		led_buf[2]=(seg_show_mode==2);
	}
	else
	{
		led_buf[0]=(parameter_index==0)?time_100ms_flag:0;
		led_buf[1]=(parameter_index==1)?time_100ms_flag:0;
		led_buf[2]=(parameter_index==2)?time_100ms_flag:0;
	}
	led_buf[3]=(ne555_frequency>parameter_frequency);
	led_buf[4]=(humidity_100x>parameter_humidity);
	led_buf[5]=(us_distance>parameter_distance);
	
	led_disp(led_buf);
	relay(relay_buf);
}
/*Motor定时器2PWM区域*/
void Timer2_Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0xFB;			//定时器时钟12T模式
	T2L = 0x9C;				//设置定时初始值
	T2H = 0xFF;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
	IE2 |= 0x04;			//使能定时器2中断
}
void Timer2_Isr(void) interrupt 12
{
	if(ne555_frequency>parameter_frequency)//80%
	{
		pwm_count=(++pwm_count)%10;
		if(pwm_count<8)
			motor(1);	
		else
			motor(0);
	}
	else
	{
		pwm_count=(++pwm_count)%10;
		if(pwm_count<2)
			motor(1);	
		else
			motor(0);
	}
}
/*Ne555定时器0区域*/
void Timer0_Init(void)		//1毫秒@12.000MHz
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
	if(++time_1s_f==100)
	{
		time_1s_f=0;
		ne555_frequency=TH0<<8|TL0;
		TH0=TL0=0;
		ne555_frequency=ne555_frequency*10;
	}
	
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	if(seg_show_mode==3)
	{
		if(++time_100ms>=100)
		{
			time_100ms=0;
			time_100ms_flag^=1;
		}
	}
	
	if(key7_press_flag==1)
	{
		if(++key7_press_1s>=1000)
			key7_press_flag=1000;
	}
	else
		key7_press_flag=0;
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
	{us_dist_task,80,0}
};
idata unsigned char task_num;
void scheduler_init(void)
{
	task_num=sizeof(SchedulerTask)/sizeof(TaskT);
}
void scheduler_run(void)
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;
		if(now_time>=SchedulerTask[i].RateTime+SchedulerTask[i].LastTime)
		{
			SchedulerTask[i].LastTime=now_time;
			SchedulerTask[i].pTaskFunc();
		}
	}
}
/*初始化区域*/
void init_task(void)
{
	unsigned char eeprom_temp;
	sys_init();
	scheduler_init();
	eeprom_read(&eeprom_temp,8,1);
	if(eeprom_temp==eeprom_check)
	{
		eeprom_read(&relay_count,0,1);
	}
	Timer2_Init();
	Timer0_Init();
	Timer1_Init();
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
