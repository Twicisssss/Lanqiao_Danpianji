/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <iic.H>
#include <onewire.H>
#include <ultrasound.H>
/*变量声明区域*/
idata unsigned long int sys_tick;
idata bit relay_buf=0;
idata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char seg_pos,seg_buf[8]={10,10,10,10,10,10,10,10};
idata unsigned char key_val,key_old,key_down,key_up;

idata unsigned char seg_show_mode=0;//【0-温度测距】【1-参数】【2-工厂】
idata bit dist_CM_M=0;//【0-CM】【1-M】
idata bit parameter_index=0;//【0-距离】【1-温度】
idata unsigned char set_mode=0;//【0-校准值】【1-介质】【2-DAC】

idata float temperature;
idata float temperature_parameter=30.0;

idata unsigned int us_velocity=340;
idata unsigned char us_distance;
idata unsigned char us_distance_parameter=40;
idata int us_distance_calibration=0;

idata float dac_voltage_out;
idata float dac_voltage_min=1.0001;

idata unsigned int time_100ms=0;
idata bit time_100ms_flag=0;

idata bit key89_press_flag=0;
idata unsigned int key89_press_2s;

pdata unsigned char distance_record[12]={0};
idata unsigned char distance_record_index=0;
idata bit record_flag=0;
idata unsigned int record_time_6s=0;

pdata float dac_output[12]={0};
idata unsigned char dac_output_index=0;
idata bit dac_output_flag=0;
idata unsigned int output_time_500ms=0;
/*数据获取区域*/
void temperature_task(void)
{
	temperature=temperature_read();
}
void adda_task(void)
{
	if(dac_output_flag==1)
	{
		if(dac_output_index<12)
		{
//			dac_output[dac_output_index]=5.0-((89.f/80.f)*(5.0-dac_voltage_min)*distance_record[dac_output_index]);
			dac_output[dac_output_index]=dac_voltage_min+((distance_record[dac_output_index]-10.0f)*((5.0f-dac_voltage_min)/80.0f));
			dac_voltage_out=dac_output[dac_output_index];
			da_write(dac_voltage_out*51.0);
		}
	}
}
void us_task(void)
{
	us_distance=((float)us_distance_get()*((float)us_velocity/340.0))+us_distance_calibration;
	
	if(record_flag==1)
	{
		if(distance_record_index<12)
		{
			distance_record[distance_record_index]=us_distance;
			distance_record_index++;
		}
	}
}
/*按键控制区域*/
void key_task(void)
{
	unsigned char i;
	key_val=key_read();
	key_down=key_val&(key_old^key_val);
	key_up=~key_val&(key_old^key_val);
	key_old=key_val;
	
	if(record_flag==1)return;
	
	if(key_old==89)
	{
		key89_press_flag=1;
	}
	if(key89_press_2s>=2000)
	{
		seg_show_mode=0;
		dist_CM_M=0;
		parameter_index=0;
		set_mode=0;
		us_distance_parameter=40;
		temperature_parameter=30.0;
		us_distance_calibration=0;
		us_velocity=340;
		dac_voltage_min=1.0;
	}
	if(key_up==89 || key_up==8 || key_up==9)
	{
		key89_press_flag=0;
		key89_press_2s=0;
	}
	
	if(key_down==4)
	{
		seg_show_mode=(++seg_show_mode)%3;
		dist_CM_M=parameter_index=set_mode=0;
	}
	else if(key_down==5)
	{
		if(seg_show_mode==0)
			dist_CM_M^=1;
		else if(seg_show_mode==1)
			parameter_index^=1;
		else if(seg_show_mode==2)
			set_mode=(++set_mode)%3;		
	}
	else if(key_down==8)
	{
		if(seg_show_mode==0)		/*【0-测距】*/
		{
			for(i=0;i<12;i++)
			{
				distance_record[i]=0;
			}
			distance_record_index=0;
			record_flag=1;				//【记录】
		}
		else if(seg_show_mode==1)		/*【1-参数】*/
		{
			if(parameter_index==0)		//【0-距离】
			{
				us_distance_parameter+=10;
				if(us_distance_parameter==100)
					us_distance_parameter=90;
			}
			else						//【1-温度】
			{
				temperature_parameter+=1.0;
				if(temperature_parameter==81.0)
					temperature_parameter=80.0;
			}
		}
		else if(seg_show_mode==2)	/*【2-工厂】*/
		{
			if(set_mode==0)				//【0-校准值】
			{
				us_distance_calibration+=5;
				if(us_distance_calibration==95)
					us_distance_calibration=90;
			}
			else if(set_mode==1)		//【1-介质】
			{
				us_velocity+=10;
				if(us_velocity==10000)
					us_velocity=9990;
			}
			else if(set_mode==2)		//【2-DAC】
			{
				dac_voltage_min+=0.1;
				if(dac_voltage_min>2.0)
					dac_voltage_min=2.0;
			}
		}
	}
	else if(key_down==9)
	{
		if(seg_show_mode==0)		/*【0-测距】*/
		{
			dac_output_flag=1;//【输出】
		}
		else if(seg_show_mode==1)		/*【1-参数】*/
		{
			if(parameter_index==0)		//【0-距离】
			{
				us_distance_parameter-=10;
				if(us_distance_parameter==0)
					us_distance_parameter=10;
			}
			else						//【1-温度】
			{
				temperature_parameter-=1.0;
				if(temperature_parameter==-1.0)
					temperature_parameter=0.0;
			}
		}
		else if(seg_show_mode==2)	/*【2-工厂】*/
		{
			if(set_mode==0)				//【0-校准值】
			{
				us_distance_calibration-=5;
				if(us_distance_calibration==-95)
					us_distance_calibration=-90;
			}
			else if(set_mode==1)		//【1-介质】
			{
				us_velocity-=10;
				if(us_velocity==0)
					us_velocity=10;
			}
			else if(set_mode==2)		//【2-DAC】
			{
				dac_voltage_min-=0.0999;
				if(dac_voltage_min<0.1)
					dac_voltage_min=0.1;
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
			seg_buf[0]=(unsigned char)(temperature)/10;
			seg_buf[1]=(unsigned char)(temperature)%10+',';
			seg_buf[2]=(unsigned int)(temperature*10)%10;
			seg_buf[3]=11;
			seg_buf[4]=us_distance/1000%10;
			seg_buf[5]=!dist_CM_M?(us_distance/100%10):(us_distance/100%10+',');
			seg_buf[6]=us_distance/10%10;
			seg_buf[7]=us_distance/1%10;
			for(j=4;j<7;j++)
			{
				if(seg_buf[j]==0 && (seg_buf[j-1]==10 || seg_buf[j-1]==11))
					seg_buf[j]=10;
			}
		break;
		case 1:
			seg_buf[0]=12;
			seg_buf[1]=(unsigned char)parameter_index+1;
			seg_buf[2]=seg_buf[3]=seg_buf[4]=seg_buf[5]=10;
			seg_buf[6]=!parameter_index?(us_distance_parameter/10):((unsigned char)temperature_parameter/10);
			seg_buf[7]=!parameter_index?(us_distance_parameter%10):((unsigned char)temperature_parameter%10);
		break;
		case 2:
			seg_buf[0]=13;
			seg_buf[1]=set_mode+1;
			seg_buf[2]=seg_buf[3]=10;
			if(set_mode==0)
			{
				seg_buf[4]=10;
				if(us_distance_calibration>=0)
				{
					seg_buf[5]=us_distance_calibration/100%10;
					seg_buf[6]=us_distance_calibration/10%10;
					seg_buf[7]=us_distance_calibration/1%10;
				}
				else
				{
					seg_buf[5]=!(us_distance_calibration<=-10 && us_distance_calibration>-100)?((-us_distance_calibration)/100%10):11;
					seg_buf[6]=!(us_distance_calibration<=-1 && us_distance_calibration>-10)?((-us_distance_calibration)/10%10):11;
					seg_buf[7]=(-us_distance_calibration)/1%10;
				}
				for(j=4;j<7;j++)
				{
					if(seg_buf[j]==0 && seg_buf[j-1]==10)
						seg_buf[j]=10;
				}
			}
			else if(set_mode==1)
			{
				seg_buf[4]=us_velocity/1000%10;
				seg_buf[5]=us_velocity/100%10;
				seg_buf[6]=us_velocity/10%10;
				seg_buf[7]=us_velocity/1%10;
				for(j=4;j<7;j++)
				{
					if(seg_buf[j]==0 && seg_buf[j-1]==10)
						seg_buf[j]=10;
				}
			}
			else if(set_mode==2)
			{
				seg_buf[4]=10;
				seg_buf[5]=10;
				seg_buf[6]=(unsigned char)dac_voltage_min%10+',';
				seg_buf[7]=(unsigned char)(dac_voltage_min*10)%10;
			}
		break;
	}
}
/*LED控制区域*/
void led_task(void)
{
	unsigned char i;
	if(seg_show_mode==0)
	{
		for(i=0;i<8;i++)
			{led_buf[i]=((us_distance>>i) & 0x01);}
	}
	else if(seg_show_mode==1)
	{
		led_buf[7]=1;
		for(i=0;i<7;i++)
			{led_buf[i]=0;}
	}
	else if(seg_show_mode==2)
	{
		led_buf[0]=time_100ms_flag;
		for(i=1;i<8;i++)
			{led_buf[i]=0;}
	}
	
	relay_buf=(((us_distance_parameter-5)<=(us_distance) && ((us_distance)<=(us_distance_parameter+5))) && (temperature<temperature_parameter));
	
	led_disp(led_buf);
	relay(relay_buf);
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
	
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	if(seg_show_mode==2)
	{
		if(++time_100ms==100)
		{
			time_100ms=0;
			time_100ms_flag^=1;
		}
	}
	else
	{
		time_100ms=0;
		time_100ms_flag=0;
	}
	
	if(key89_press_flag)
	{
		if(++key89_press_2s>=2000)
			key89_press_2s=2001;
	}
	else
		key89_press_2s=0;
	
	
	if(record_flag==1)
	{
		if(++record_time_6s==6000)
		{
			record_flag=0;
			record_time_6s=0;
			distance_record_index=0;
		}
	}
	
	if(dac_output_flag==1)
	{
		if(++output_time_500ms==500)
		{
			output_time_500ms=0;
			if(dac_output_index<12)
			{
				dac_output_index++;
			}
			else
			{
				dac_output_flag=0;
				output_time_500ms=0;
				dac_output_index=0;
			}
		}
	}
}
/*调度器区域*/
typedef struct{
	void (*pTaskFunc)(void);
	unsigned long int RateMs;
	unsigned long int LastTime;
}TaskT;
idata TaskT scheduler_task[]={
	{key_task,10,0},
	{seg_task,50,0},
	{led_task,1,0},
	{temperature_task,300,0},
	{adda_task,50,0},
	{us_task,450,0}
	
};
idata unsigned char task_num;
void scheduler_init(void)
{
	task_num=sizeof(scheduler_task)/sizeof(TaskT);
}
void scheduler_run(void)
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;
		if(now_time>=scheduler_task[i].RateMs+scheduler_task[i].LastTime)
		{
			scheduler_task[i].LastTime=now_time;
			scheduler_task[i].pTaskFunc();
		}
	}
}
/*初始化区域*/
void init_task(void)
{
	sys_init();
	scheduler_init();
	while(temperature_read()==85);
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
