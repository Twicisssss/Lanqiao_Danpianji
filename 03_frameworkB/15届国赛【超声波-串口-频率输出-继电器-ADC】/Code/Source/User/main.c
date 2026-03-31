/*头文件引用区域*/
#include <STC15F2K60S2.H>
#include <init.H>
#include <led.H>
#include <seg.H>
#include <key.H>
#include <uart.H>
#include <ultrasound.H>
#include <iic.H>
#include <math.H>

/*变量声明区域*/
idata unsigned long int sys_tick;
pdata unsigned char uart_buf[10]={0};
idata unsigned char uart_rx_index;
pdata unsigned char uart_rx_flag;
idata unsigned char uart_rx_tick;
idata unsigned char seg_pos;
idata unsigned char seg_buf[8]={10,10,10,10,10,10,10,10};
pdata unsigned char led_buf[8]={0,0,0,0,0,0,0,0};
idata unsigned char key_val,key_old,key_down,key_up;
idata unsigned char seg_show_mode=0;//【0-坐标】【1-速度】【2-参数】

idata unsigned int ne555_frequency;
idata unsigned int ne555_cycle_1s;
idata unsigned char device_status=1;				//【0-运行】【1-空闲】【2-等待】
idata float device_speed;							//设备运行速度，保留1位小数，单位为cm/s，不考虑负数情况
pdata float speed_parameter_R=1.0;					//可调参数R：1.0~2.0 (cm/s)/Hz
pdata char speed_parameter_B=0;						//可调参数B：-90~90 cm/s
idata bit para_RB_index;							//【0-R】【1-B】
pdata float par_R_set=1.0;							//参数R设置参数：1.0~2.0 (cm/s)/Hz
pdata char par_B_set=0;								//参数B设置参数：-90~90 cm/s
pdata unsigned int device_location[2]={0,0};		//设备位置坐标{x,y}
pdata unsigned int destination_location[2]={0,0};	//目的地坐标{x,y}
idata bit destination_get_flag;						//接收到目的地坐标标志位【0-未收到】【1-接收到】

idata int displacement_length;
idata float speed_x,speed_y;
idata float dis_x,dis_y;
idata unsigned char us_distance;
//idata bit obstacle_flag;	//存在障碍物标志位【0-不存在】【1-存在】

idata float ad_light_voltage;
idata unsigned char day_night_status;//【0-夜间】【1-日间】

idata unsigned char led_flash_time_100ms;
idata bit led_flash_flag;

pdata unsigned int time_3000ms;
idata bit arrived_flag;
/*数据获取区域*/
void us_dist_task(void)
{
	us_distance=us_distance_get();
	if(device_status==0 && us_distance<=30)	//【0-运行】状态下，若距离小于30cm
	{
//		obstacle_flag=1;
		device_status=2;					//设备自动切换到【2-等待】状态
	}
}
void adda_task(void)
{
	ad_light_voltage=ad_read(0x01)/51.0;
	day_night_status=(ad_light_voltage>1.2);//分压结果大于1.2V时，为日间场景，否则为夜间场景。
}
void device_move_task(void)
{
	if(device_status==0)
	{
		dis_x=fabs((int)device_location[0]-(int)destination_location[0]);
		dis_y=fabs((int)device_location[1]-(int)destination_location[1]);
		displacement_length=sqrt(pow(dis_x,2)+pow(dis_y,2));//计算需要位移的距离
		speed_x=device_speed*(dis_x/displacement_length);
		speed_y=device_speed*(dis_y/displacement_length);
		if(destination_get_flag==1)
		{
			if(destination_location[0]>device_location[0])
				device_location[0]+=speed_x/10.0;
			else if(destination_location[0]<device_location[0] && device_location[0]>=0)
				device_location[0]-=speed_x/10.0;
			dis_x-=speed_x/10.0;
			
			if(destination_location[1]>device_location[1])
				device_location[1]+=speed_y/10.0;
			else if(destination_location[1]<device_location[1] && device_location[1]>=0)
				device_location[1]-=speed_y/10.0;
			dis_y-=speed_y/10.0;
			
			if(dis_x<=speed_x/10.0 && dis_y<=speed_y/10.0)
			{
				device_status=1;//【1-空闲】
				displacement_length=0;
				dis_x=dis_y=0;
				device_speed=speed_x=speed_y=0;
				destination_get_flag=0;
				arrived_flag=1;
				device_location[0]=destination_location[0];
				device_location[1]=destination_location[1];
			}
		}
	}
}
/*按键控制区域*/
void key_task(void)
{
	key_val=key_read();
	key_down=key_val&(key_old^key_val);
	key_up=~key_val&(key_old^key_val);
	key_old=key_val;
	
	if(key_down==6)
	{
		destination_location[0]=100;
		destination_location[1]=100;
		destination_get_flag=1;
	}
	
	switch(key_down)
	{
		case 4:
			//【0-运行】【1-空闲】【2-等待】
			switch(device_status)
			{
				case 1:
					if(destination_get_flag==1)	//若已收到目的地坐标，按下S4按键
					{
//						dis_x=fabs((int)device_location[0]-(int)destination_location[0]);
//						dis_y=fabs((int)device_location[1]-(int)destination_location[1]);
//						displacement_length=sqrt(pow(dis_x,2)+pow(dis_y,2));//计算需要位移的距离
//						speed_x=device_speed*(dis_x/displacement_length);
//						speed_y=device_speed*(dis_y/displacement_length);
						device_status=0;		//切换为【0-运行】状态
					}
				break;
				case 2:
					if(us_distance>30)//若"障碍物"已清除，按下S4按键
						device_status=0;//切换为【0-运行】状态
				break;
				case 0:
					device_status=2;//切换为【2-等待】状态
				break;
			}
		break;
		case 5://在【1-空闲】状态下，强制将设备当前位置重置为（0,0），其它状态下，S5按键无效
			if(device_status==1)
			{
				device_location[0]=0;
				device_location[1]=0;
				destination_location[0]=0;
				destination_location[1]=0;
			}
		break;
		case 8://"数码管显示界面切换"按键
			seg_show_mode=(++seg_show_mode)%3;
			if(seg_show_mode==2)
			{
				para_RB_index=0;
				par_R_set=speed_parameter_R;
				par_B_set=speed_parameter_B;
			}
			if(seg_show_mode==0)
			{
				speed_parameter_R=par_R_set;
				speed_parameter_B=par_B_set;
			}
		break;
		case 9:
			if(seg_show_mode==2)
				para_RB_index^=1;
		break;
		case 12://"加"按键
			if(seg_show_mode==2)
			{
				if(para_RB_index==0)//R
				{
					par_R_set+=0.1;
					if(par_R_set>2.0)
						par_R_set=2.0;
				}
				else//B
				{
					par_B_set+=5;
					if(par_B_set>90)
						par_B_set=90;
					
				}
			}
		break;
		case 13://"减"按键
			if(seg_show_mode==2)
			{
				if(para_RB_index==0)//R
				{
					par_R_set-=0.1;
					if(par_R_set<1.0)
						par_R_set=1.0;
				}
				else//B
				{
					par_B_set-=5;
					if(par_B_set<-90)
						par_B_set=-90;
				}
			}
		break;
	}
}
/*数码管控制区域*/
void seg_task(void)
{
	unsigned char j;
	switch(seg_show_mode)
	{
		case 0://【0-坐标】
			seg_buf[0]=12;
			if(device_status==0 || device_status==2)//【0-运行】或【2-等待】状态下，显示目的地X、丫坐标
			{
				seg_buf[1]=destination_location[0]/100%10;
				seg_buf[2]=destination_location[0]/10%10;
				seg_buf[3]=destination_location[0]/1%10;
				seg_buf[4]=11;
				seg_buf[5]=destination_location[1]/100%10;
				seg_buf[6]=destination_location[1]/10%10;
				seg_buf[7]=destination_location[1]/1%10;
			}
			else//【1-空闲】状态下，显示设备当前位置X、Y坐标
			{
				seg_buf[1]=device_location[0]/100%10;
				seg_buf[2]=device_location[0]/10%10;
				seg_buf[3]=device_location[0]/1%10;
				seg_buf[4]=11;
				seg_buf[5]=device_location[1]/100%10;
				seg_buf[6]=device_location[1]/10%10;
				seg_buf[7]=device_location[1]/1%10;
			}
			for(j=1;j<3;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]>=10)
					seg_buf[j]=10;
			}		
			for(j=5;j<7;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]>=10)
					seg_buf[j]=10;
			}		
		break;
		case 1://【1-速度】
			seg_buf[0]=13;
			seg_buf[1]=device_status+1;
			seg_buf[2]=10;
			if(device_status==0)//【0-运行】
			{
				seg_buf[3]=(unsigned int)(device_speed)/1000%10;
				seg_buf[4]=(unsigned int)(device_speed)/100%10;
				seg_buf[5]=(unsigned int)(device_speed)/10%10;
				seg_buf[6]=(unsigned int)(device_speed)/1%10+',';
				seg_buf[7]=(unsigned int)(device_speed*10)%10;
			}
			else if(device_status==1)//【1-空闲】
			{
				seg_buf[3]=seg_buf[4]=seg_buf[5]=seg_buf[6]=seg_buf[7]=11;
//				seg_buf[3]=ne555_frequency/10000%10;
//				seg_buf[4]=ne555_frequency/1000%10;;
//				seg_buf[5]=ne555_frequency/100%10;
//				seg_buf[6]=ne555_frequency/10%10;
//				seg_buf[7]=ne555_frequency/1%10;
			}
			else if(device_status==2)//【2-等待】
			{
				seg_buf[3]=us_distance/10000%10;
				seg_buf[4]=us_distance/1000%10;
				seg_buf[5]=us_distance/100%10;
				seg_buf[6]=us_distance/10%10;
				seg_buf[7]=us_distance/1%10;
			}
			for(j=3;j<7;j++)
			{
				if(seg_buf[j]==0 && seg_buf[j-1]>=10)
					seg_buf[j]=10;
			}			
		break;
		case 2://【2-参数】
			seg_buf[0]=14;
			seg_buf[1]=10;
			seg_buf[2]=(unsigned char)(par_R_set)%10+',';
			seg_buf[3]=(unsigned char)(par_R_set*10)%10;
			seg_buf[4]=10;
			if(par_B_set>=100 && par_B_set<=999)		//100 <= P <= 999
			{
				seg_buf[5]=par_B_set/100%10;
				seg_buf[6]=par_B_set/10%10;
				seg_buf[7]=par_B_set/1%10;
			}
			else if(par_B_set>=10 && par_B_set<=99)	//10 <= P <= 99
			{
				seg_buf[5]=10;
				seg_buf[6]=par_B_set/10%10;
				seg_buf[7]=par_B_set/1%10;
			}
			else if(par_B_set>=0 && par_B_set<=9)		//0 <= P <= 9
			{
				seg_buf[5]=10;
				seg_buf[6]=10;
				seg_buf[7]=par_B_set/1%10;
			}
			else if(par_B_set>=-9 && par_B_set<=-1)	//-9 <= P <= -1
			{
				seg_buf[5]=10;
				seg_buf[6]=11;
				seg_buf[7]=(-par_B_set)/1%10;
			}
			else if(par_B_set>=-99 && par_B_set<=-10)	//-99 <= P <= -10
			{
				seg_buf[5]=11;
				seg_buf[6]=(-par_B_set)/10%10;
				seg_buf[7]=(-par_B_set)/1%10;
			}
		break;
		
	}
}
/*LED控制区域*/
void led_task(void)
{
	if(device_status==1)		//【1-空闲】状态：L1熄灭
		led_buf[0]=0;
	else if(device_status==0)	//【0-运行】状态：L1点亮
		led_buf[0]=1;
	else if(device_status==2)	//【2-等待】状态：L1闪烁
		led_buf[0]=led_flash_flag;
	
	
	if(device_status==0)//【0-运行】状态下：【1-日间】L2熄灭，【0-夜间】L2点亮。
		led_buf[1]=!day_night_status;
	else				//【1-空闲】【2-等待】状态下：L2 熄灭
		led_buf[1]=0;

	
	led_buf[2]=arrived_flag;
	
	led_disp(led_buf);
	relay(device_status==0);
}
/*串口接收区域*/
void uart_task(void)
{
	unsigned int x,y;// 用于存储坐标{x,y}
	int chars_read;  // 用于存储已读取的字符数
	if(uart_rx_index==0)return;
	if(uart_rx_tick>=10)
	{
		int result = sscanf(uart_buf, "(%u,%u)%n", &x, &y, &chars_read);
		uart_rx_flag=0;
		uart_rx_tick=0;
		
		//device_status:【0-运行】【1-空闲】【2-等待】		
		// 验证：(成功解析两个数字) && (读取的字符数等于输入字符串长度)
		if(result == 2 && chars_read == strlen(uart_buf))
		{
			if(device_status==1)
			{
				destination_get_flag=1;
				destination_location[0]=x;
				destination_location[1]=y;
				printf("Got it");
//				printf("(%u,%u)\n",destination_location[0],destination_location[1]);
//				printf("设备状态=%bu\n",device_status);
//				printf("获取目的地=%bu\n",(unsigned char)destination_get_flag);
//				printf("设备速度=%f\n",device_speed);
//				printf("x轴相距=%f\n",dis_x);
//				printf("y轴相距=%f\n",dis_y);
			}
			else
			{
				printf("Busy");
			}
		}
		
		else if (strcmp(uart_buf, "?") == 0)
		{
			if(device_status==1)
				printf("Idle");
			else if(device_status==2)
				printf("Wait");
			else if(device_status==0)
				printf("Busy");
		}	
		else if (strcmp(uart_buf, "#") == 0)
		{
			printf("(%u,%u)",device_location[0],device_location[1]);
//			printf("设备状态=%bu\n",device_status);
//			printf("获取目的地=%bu\n",(unsigned char)destination_get_flag);
//			printf("设备速度=%f\n",device_speed);
//			printf("x轴相距=%f\n",dis_x);
//			printf("y轴相距=%f\n",dis_y);
		}
		else
		{
			printf("Error");
		}
		memset(uart_buf,0,uart_rx_index);
		uart_rx_index=0;
	}
}
/*NE555定时器0区域*/
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
	EA=1;
}
void Timer1_Isr(void) interrupt 3
{
	sys_tick++;
	
	if(++ne555_cycle_1s==100)
	{
		ne555_cycle_1s=0;
		ne555_frequency=TH0<<8|TL0;
		ne555_frequency=ne555_frequency*10;
		device_speed=(3.14*speed_parameter_R*ne555_frequency)/100.0+speed_parameter_B;
//		device_speed=(3.00*speed_parameter_R*ne555_frequency)/100.0+speed_parameter_B;
		TH0=TL0=0;
	}
	
	if(uart_rx_flag)
		uart_rx_tick++;
	
	seg_pos=(++seg_pos)%8;
	if(seg_buf[seg_pos]>20)
		seg_disp(seg_pos,seg_buf[seg_pos]-',',1);
	else
		seg_disp(seg_pos,seg_buf[seg_pos],0);
	
	if(device_status==2)
	{
		if(++led_flash_time_100ms==100)
		{
			led_flash_time_100ms=0;
			led_flash_flag^=1;
		}
	}
	else
		led_flash_time_100ms=led_flash_flag=0;
	
	
	if(arrived_flag==1)
	{
		if(++time_3000ms>=3000)
		{
			arrived_flag=0;
			time_3000ms=0;
		}
	}
}
/*串口中断区域*/
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		uart_rx_tick=0;
		uart_rx_flag=1;
		uart_buf[uart_rx_index++]=SBUF;
		RI = 0;			//清除串口1接收中断请求位
		if (uart_rx_index > 10)
		{
		  uart_rx_index = 0; // 防止溢出
		  memset(uart_buf, 0, 10);
		}
	}
}
/*调度器区域*/
typedef struct{
	void(*task_function)(void);
	unsigned long int rate_time;
	unsigned long int last_time;
}Task_Message;

idata Task_Message schedule_task[]={
	{led_task,1,0},
	{seg_task,200,0},
	{key_task,10,0},
	{uart_task,10,0},
	{adda_task,160,0},
	{us_dist_task,100,0},
	{device_move_task,100,0},
};

idata unsigned char task_num;
void schedule_init(void)
{
	task_num=sizeof(schedule_task)/sizeof(Task_Message);
}

void schedule_run(void)
{
	unsigned char i;
	for(i=0;i<task_num;i++)
	{
		unsigned long int now_time=sys_tick;
		if(now_time>=(schedule_task[i].rate_time+schedule_task[i].last_time))
		{
			schedule_task[i].last_time=now_time;
			schedule_task[i].task_function();
		}
	}
}
/*初始化区域*/
void init_task(void)
{
	sys_init();
	schedule_init();
	Timer0_Init();
	Uart1_Init();
	Timer1_Init();
}
/*主函数区域*/
void main()
{
	init_task();
	while(1)
	{
		schedule_run();
	}
}
