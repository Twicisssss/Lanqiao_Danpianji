/*====================头文件引用区域====================*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <ds1302.H>
#include <iic.H>

/*====================变量声明区域====================*/
idata unsigned long int sys_tick;
idata unsigned char seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char seg_pos;
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char key_val,key_down,key_up,key_old;

idata unsigned char seg_show_mode=0;//【0-频率】【1-参数】【2-时间】【3-回显】

idata unsigned int ne555_frequency;
idata unsigned char ne555_cycle_1s;
idata long int correct_frequency;
idata unsigned int parameter_f_overrun=2000;	//频率超限参数：2000Hz	【1000~9000】
idata int parameter_f_calibration=0;			//频率校准值参数：0Hz	【-900~900】
idata bit parameter_index;					//【0-超限参数】【1-校准值参数】

idata unsigned char rtc[3]={23,59,55};

idata bit echo_index;								//【0-频率回显】【1-时间回显】
idata unsigned int frequency_max_echo;			//最大频率值		【0-频率回显】
idata unsigned char f_max_time_echo[3]={0,0,0};	//最大频率发生时间	【1-时间回显】

idata float da_voltage;

idata unsigned char L1_time_200ms;
idata bit L1_time_200ms_flag;

idata unsigned char L2_time_200ms;
idata bit L2_time_200ms_flag;
/*====================数据获取区域====================*/
void rtc_task(void)
{
	read_rtc(rtc);
	if(frequency_max_echo<=correct_frequency)
	{
		frequency_max_echo=correct_frequency;
		f_max_time_echo[0]=rtc[0];
		f_max_time_echo[1]=rtc[1];
		f_max_time_echo[2]=rtc[2];
	}
}
void adda_task(void)
{
	if(correct_frequency<0)
		da_voltage=0.0;
	else if(correct_frequency>=parameter_f_overrun)
		da_voltage=5.0;
	else if(correct_frequency<=500 && correct_frequency>=0)
		da_voltage=1.0;
	else
		da_voltage=(4.0/(parameter_f_overrun-500.0))*(correct_frequency-500.0)+1.0;
		
	da_write(da_voltage*51);
}
/*====================按键控制区域====================*/
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
			parameter_index=echo_index=0;
		break;
		case 5:
			if(seg_show_mode==1)		//【1-参数】
				parameter_index^=1;
			else if(seg_show_mode==3)	//【3-回显】
				echo_index^=1;
		break;
		case 8://加
			if(seg_show_mode==1)//【1-参数】
			{
				if(parameter_index==0)//【0-超限参数】
				{
					parameter_f_overrun+=1000;
					if(parameter_f_overrun>9000)
						parameter_f_overrun=9000;
				}
				else//【1-校准值参数】
				{
					parameter_f_calibration+=100;
					if(parameter_f_calibration>900)
						parameter_f_calibration=900;
				}
			}
		break;
		case 9://减
			if(seg_show_mode==1)//【1-参数】
			{
				if(parameter_index==0)//【0-超限参数】
				{
					parameter_f_overrun-=1000;
					if(parameter_f_overrun<1000)
						parameter_f_overrun=1000;
				}
				else//【1-校准值参数】
				{
					parameter_f_calibration-=100;
					if(parameter_f_calibration<-900)
						parameter_f_calibration=-900;
				}
			}
		break;
	}
}
/*====================数码管控制区域====================*/
void seg_task(void)
{
	unsigned char i,j;
	switch(seg_show_mode)
	{
		case 0:
			seg_buf[0]=12;
			seg_buf[1]=10;
			seg_buf[2]=10;
			if(correct_frequency>=0)
			{
				seg_buf[3]=correct_frequency/10000%10;
				seg_buf[4]=correct_frequency/1000%10;
				seg_buf[5]=correct_frequency/100%10;
				seg_buf[6]=correct_frequency/10%10;
				seg_buf[7]=correct_frequency/1%10;
				for(i=3;i<7;i++)
				{
					if(seg_buf[i]==0 && seg_buf[i-1]>=10)
						seg_buf[i]=10;
				}
			}
			else
			{
				seg_buf[3]=10;
				seg_buf[4]=10;
				seg_buf[5]=10;
				seg_buf[6]=16;
				seg_buf[7]=16;
			}
		break;
		case 1:
			seg_buf[0]=13;
			seg_buf[1]=(unsigned char)(parameter_index)+1;
			seg_buf[2]=10;
			seg_buf[3]=10;
			if(parameter_index==0)
			{
				seg_buf[4]=parameter_f_overrun/1000%10;
				seg_buf[5]=parameter_f_overrun/100%10;
				seg_buf[6]=parameter_f_overrun/10%10;
				seg_buf[7]=parameter_f_overrun/1%10;
			}
			else
			{
				if(parameter_f_calibration>=0)
				{
					seg_buf[4]=parameter_f_calibration/1000%10;
					seg_buf[5]=parameter_f_calibration/100%10;
					seg_buf[6]=parameter_f_calibration/10%10;
					seg_buf[7]=parameter_f_calibration/1%10;
				}
				else
				{
					seg_buf[4]=(-parameter_f_calibration)/1000%10;
					seg_buf[5]=(-parameter_f_calibration)/100%10;
					seg_buf[6]=(-parameter_f_calibration)/10%10;
					seg_buf[7]=(-parameter_f_calibration)/1%10;
					for(j=4;j<7;j++)
					{
						if(seg_buf[j]==0 && seg_buf[j-1]==10 && seg_buf[j+1]<=9)
							seg_buf[j]=11;
					}
				}
			}
			for(i=4;i<7;i++)
			{
				if(seg_buf[i]==0 && seg_buf[i-1]>=10)
					seg_buf[i]=10;
			}
		break;
		case 2:
			seg_buf[0]=rtc[0]/10;
			seg_buf[1]=rtc[0]%10;
			seg_buf[2]=11;
			seg_buf[3]=rtc[1]/10;
			seg_buf[4]=rtc[1]%10;
			seg_buf[5]=11;
			seg_buf[6]=rtc[2]/10;
			seg_buf[7]=rtc[2]%10;
		break;		
		case 3:
			seg_buf[0]=14;
			seg_buf[1]=!echo_index?12:15;
			if(echo_index==0)
			{
				seg_buf[2]=10;
				seg_buf[3]=frequency_max_echo/10000%10;
				seg_buf[4]=frequency_max_echo/1000%10;
				seg_buf[5]=frequency_max_echo/100%10;
				seg_buf[6]=frequency_max_echo/10%10;
				seg_buf[7]=frequency_max_echo/1%10;
				for(i=3;i<7;i++)
				{
					if(seg_buf[i]==0 && seg_buf[i-1]>=10)
						seg_buf[i]=10;
				}
			}
			else
			{
				seg_buf[2]=f_max_time_echo[0]/10;
				seg_buf[3]=f_max_time_echo[0]%10;
				seg_buf[4]=f_max_time_echo[1]/10;
				seg_buf[5]=f_max_time_echo[1]%10;
				seg_buf[6]=f_max_time_echo[2]/10;
				seg_buf[7]=f_max_time_echo[2]%10;
			}
		break;		
	}
}
/*====================led控制区域====================*/
void led_task(void)
{
	led_buf[0]=seg_show_mode==0?L1_time_200ms_flag:0;
	
	led_buf[1]=correct_frequency>0?L2_time_200ms_flag:1;
	
	led_disp(led_buf);
}
/*====================NE555定时器0区域====================*/
void NE555_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
/*====================定时器1区域====================*/
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
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	if(++ne555_cycle_1s==100)
	{
		ne555_cycle_1s=0;
		ne555_frequency=TH0<<8|TL0;
		ne555_frequency=ne555_frequency*10;
		correct_frequency=(long int)ne555_frequency+parameter_f_calibration;
		TH0=TL0=0;
	}
	
	if(seg_show_mode==0)
	{
		if(++L1_time_200ms==200)
		{
			L1_time_200ms=0;
			L1_time_200ms_flag^=1;
		}
	}
	else
		L1_time_200ms=L1_time_200ms_flag=0;
	
	
	
	if(correct_frequency>parameter_f_overrun)
	{
		if(++L2_time_200ms==200)
		{
			L2_time_200ms=0;
			L2_time_200ms_flag^=1;
		}
	}
	else
	{
		L2_time_200ms=L2_time_200ms_flag=0;
	}
}
/*====================调度器区域====================*/
typedef struct{
	void (*task_function)(void);
	unsigned long int rate_time;
	unsigned long int last_time;
}TaskMessage;
idata TaskMessage ScheduleTask[]={
	{key_task,20,0},
	{led_task,1,0},
	{seg_task,200,0},
	{rtc_task,300,0},
	{adda_task,250,0},
};
idata unsigned char task_num;
void schedule_init(void)
{
	task_num=sizeof(ScheduleTask)/sizeof(TaskMessage);
}
void schedule_run(void)
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;
		if(now_time>=ScheduleTask[i].rate_time+ScheduleTask[i].last_time)
		{
			ScheduleTask[i].last_time=now_time;
			ScheduleTask[i].task_function();
		}
	}
}
/*====================初始化区域====================*/
void init_task(void)
{
	sys_init();
	schedule_init();
	set_rtc(rtc);
	NE555_Init();
	Timer1_Init();
}
/*====================主函数区域====================*/
void main(void)
{
	init_task();
	while(1)
	{
		schedule_run();
	}
}

