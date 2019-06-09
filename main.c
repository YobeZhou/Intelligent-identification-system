/*
	*Project: Smart Check-in/out SYS V1.1
	*Basic Function: 1.three mode: standard mode/ auto add ID mode/ auto delete ID mode
					 2.base on kernel_list to management ID information
					 3.read card and match ID info will show the exact picture.
	*DATE: 02/05/2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "api_v4l2.h"
#include "rfid.h"
#include "lcd.h"

#define CAM_DEV     "/dev/video9"

unsigned char frame_buffer[IMAGEHEIGHT*IMAGEWIDTH*3];
FrameBuffer *cam_buffer;
volatile int g_SYS_Ctrl_Flag = g_SYS_Stand;
volatile int uart_fd=0;


pthread_t rfid_t, sys_t,cam_t;
pthread_mutex_t wr_lock;
pthread_mutex_t rd_lock;

void * Sys_Work(void * arg)
{	
	while(g_SYS_Ctrl_Flag!=g_SYS_EXIT)
	{
		pthread_mutex_lock(&rd_lock);
		switch(g_SYS_Ctrl_Flag)
		{
			case g_ID_IN:
				rfid_add_id();
				break;
				
			case g_ID_OUT:
				rfid_del_id();
				break;
				
			case g_SYS_Stand:
				rfid_standard_mode();
				break;
				
			default:
				break;
				
		}
		pthread_mutex_unlock(&wr_lock);
	}
	pthread_exit(Sys_Work);
}

void * rfid_work(void * arg)
{
	unsigned int id_num=0;
	int ret;

	while(g_SYS_Ctrl_Flag != g_SYS_EXIT)
	{
		pthread_mutex_lock(&wr_lock);
		bzero(buf, 32);
		ret = read(uart_fd , buf, 32);
		if(0 >= ret)
		{
			perror("read RFID failed");
			break;
		}
		if(ret == 12)
		{
			id_num = Hex_Num(buf);
			printf("got id is:%u\n",id_num);
		}
		else
		{
			printf("/*****Id type error,Please check your devices!*******/\n");
			g_SYS_Ctrl_Flag=g_SYS_EXIT;
			ret = 0;
			break;
		}
		pthread_mutex_unlock(&rd_lock);
	}
	pthread_exit(rfid_work);
} 

void *Cam_Work(void * arg)
{
	int x=0,y=0;
	unsigned int blue, green, red;
	int color;
	char *c_buf;

	cam_buffer=calloc(1,sizeof(FrameBuffer));
	linux_v4l2_device_init(CAM_DEV);
    linux_v4l2_start_capturing();

	while(g_SYS_Ctrl_Flag !=g_SYS_EXIT)
	{
		linux_v4l2_get_fream(cam_buffer);
		if(g_SYS_Ctrl_Flag==g_ID_IN)
		{
			yuyv_2_rgb888(cam_buffer);
			c_buf = frame_buffer;
			for(y=OFFSET_Y;y < (IMAGEHEIGHT+OFFSET_Y); y++)
			{
				for (x=OFFSET_X;x < (IMAGEWIDTH+OFFSET_X); x++)
				{
					red   = *c_buf++;
					green = *c_buf++;
					blue  = *c_buf++;
			
					color = red << 16 | green << 8 | blue << 0;
					lcd_draw_point(x, y, color);				
				}
			}
		}
	}	
}

int main(int argc, char **argv)
{
	int ret;
	int x=0,y=0;
	volatile int flag;
	
	lcd_open();
	lcd_init(0,0,Color_Black);
	uart2_init();
	
	head = init_list();
	rfid_info_init();
	
	pthread_mutex_init(&wr_lock, NULL);
	pthread_mutex_init(&rd_lock, NULL);
	pthread_mutex_lock(&rd_lock);

	
	if( (ret = pthread_create(&cam_t , NULL , Cam_Work, NULL)) != 0)
	{
		printf("Camera Work thread failed \n");
		return -1;
	}
		
	if( (ret = pthread_create(&rfid_t , NULL , rfid_work, NULL)) != 0)
	{
		printf("rfid_work thread failed \n");
		return -1;
	}
	
	if( (ret = pthread_create(&sys_t , NULL , Sys_Work, NULL)) != 0)
	{
		printf("System Work thread failed \n");
		return -1;
	}

	
	sleep(1);
	
	printf("You can select working mode:\n");
	printf("select 1:Add ID in SYS\n");
	printf("select 2:DEL ID in SYS\n");
	printf("select 3:Standard Mode\n");
	printf("select 0:SYS EXIT\n");

	while(g_SYS_Ctrl_Flag!=g_SYS_EXIT)
	{
		scanf("%d",&flag);
		
		if(flag==1)
		{
			g_SYS_Ctrl_Flag = g_ID_IN;
			printf("System ID Add mode\n");
		}
		else if(flag==2)
		{
			g_SYS_Ctrl_Flag = g_ID_OUT;
			printf("ID Delete mode\n");
		}	
		else if(flag==3)
		{
			g_SYS_Ctrl_Flag = g_SYS_Stand;
			printf("System standard mode\n");
		}
		else if(flag == 0)
		{
			g_SYS_Ctrl_Flag = g_SYS_EXIT;
			printf("System down\n");
			lcd_init(0,0,Color_Black);
			free(head);
			pthread_cancel(rfid_t);
			pthread_cancel(sys_t);
			pthread_cancel(cam_t);
		}
		else 
			printf("/**************input error*****************/");
	
		while(getchar()!='\n');
	}
		
	lcd_close();
	close(uart_fd);
	
	return 0;
}
