#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <termios.h>

#include "rfid.h"
#include "lcd.h"
#include "kernel_list.h"



struct RFID_table *head;
char buf[32]={0};




struct RFID_table
{
	unsigned int id_num;
	//char uid[4];
	char file_name[64];
	struct list_head list;
};

struct RFID_table *init_list(void)
{
	struct RFID_table *head = malloc(sizeof(struct RFID_table));
	if(head != NULL)
	{
		INIT_LIST_HEAD(&head->list);
	}
	return head;
}


void uart2_init(void)
{
	uart_fd = open(UART1_DEV, O_RDWR);
	if(uart_fd == -1)
	{
		perror("open failed in rfid");
		return;
	}
	struct termios termios_new;
	bzero( &termios_new, sizeof(termios_new));

	cfmakeraw(&termios_new);

	termios_new.c_cflag=(B9600);
	termios_new.c_cflag |= CLOCAL | CREAD;
	
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8;

	termios_new.c_cflag &= ~PARENB;

	termios_new.c_cflag &= ~CSTOPB;

	tcflush( uart_fd,TCIFLUSH);
	termios_new.c_cc[VTIME] = 0;
	termios_new.c_cc[VMIN] = 4;
	tcsetattr(uart_fd ,TCSANOW,&termios_new);
}

unsigned int Hex_Num(char *buffer)
{
	int i,j;
	char temp[4]={0};
	unsigned int id_num=0;
	for(i=7,j=0;i<11;i++,j++)
	{
		temp[j] = *(buffer+i);//0xaa 0xbb 0xcc 0xdd
		id_num = id_num | temp[j]<<8*j;
	}
	return id_num;	
}

void show_Txt_info(FILE *fp)
{
	int i,t_temp;
	char t_file_name[64];
	
	int num = 0;
	while((t_temp = getc(fp)) != EOF)
	{
		if(t_temp == '\n')
			num++;
	}
	if(num==0)
	{
		printf("/*************New list info,Please insert ID***********/\n");
		return;
	}	
	fseek(fp,0,SEEK_SET);
	for(i=0;i<num;i++)
	{
		fscanf(fp, "%u", &t_temp);			
		printf("Id:%u ",t_temp);
			
		fscanf(fp, "%s", t_file_name);
		printf("[%d] file_name: %s\n",i,t_file_name);
	}
	printf("/****************Show txt info OK!****************/\n");
}

void rfid_info_init(void)
{
	int i=0, j=0;
	int info_temp;
	FILE * fp;
	int num=0;
	
	fp = fopen("./RFID_link.txt", "a+");
	if(fp == NULL)
	{
		perror("open RFID_link.txt \n");
		return ;
	}
	while((info_temp = getc(fp)) != EOF)
	{
		if(info_temp == '\n')
			num++;
	}
	if(num==0)
	{
		printf("/***********New list info,Please insert ID*************/\n");
		fclose(fp);
		return;
	}	
	fseek(fp,0,SEEK_SET);
	for(i=0;i<num;i++)
	{
		struct RFID_table *new = calloc(1,sizeof(struct RFID_table));
		fscanf(fp, "%u", &(new->id_num));			
		printf("Id:%u ",new->id_num);
			
		fscanf(fp, "%s", new->file_name);
		printf(" file_name:%s\n",new->file_name);
		list_add_tail(&new->list,&head->list);
	}
	fclose(fp);
	printf("/*************ID infomation init ok!************/\n");
}

void  rfid_add_id(void)
{
	int ret;
	int i=0,j;
	
	unsigned int id_num=0;
	char file_name[64]={0};
	char write_buf[100]={0};
	struct list_head *pos;
	struct RFID_table *p;
	FILE * fp;	
	

	id_num = Hex_Num(buf);
	if(list_empty(&head->list)==1)
	{
		printf("/*****Got New ID,Thank you for Join Us!****/\n");
		bzero(file_name, 64);
		bzero(write_buf,100);
		struct RFID_table *new = calloc(1,sizeof(struct RFID_table));
		
		new->id_num = id_num;
		sprintf(file_name,"%u.jpg",id_num);
		strcpy(new->file_name,file_name);
		sprintf(write_buf,"%u %u.jpg\n",id_num,id_num);
		list_add_tail(&new->list,&head->list);
		fp = fopen("./RFID_link.txt", "a+");
		if(fp == NULL)
		{
			perror("open RFID_link.txt \n");
			return;
		}
		fwrite(write_buf,strlen(write_buf),1,fp);
		
		fseek(fp,0,SEEK_SET);
		show_Txt_info(fp);
		fclose(fp);
		usleep(500);
		encode_jpeg(frame_buffer,file_name,IMAGEWIDTH,IMAGEHEIGHT); //RGB24 to Jpeg 
		
		
		printf("/***********ID Add Success,You can Add Another ID*********/\n");
		printf("/***************Change Mode Please Select: ***************/\n");
		printf("/***************select 2:DEL ID in SYS********************/\n");
		printf("/***************select 3:Back To Standard Mode************/\n");
		printf("/***************select 0:SYS EXIT*************************/\n");
	}
	else
	{
		list_for_each(pos,&head->list)
		{
			p = list_entry(pos,struct RFID_table,list);
			i++;
			if(p->id_num == id_num)	
			{
				printf("/************Your ID Has Included,Try Next ID!************/\n");
				i=0;
				break;
			}	
		}
		if(i!=0)
		{
			printf("/*****Got New ID,Thank you for Join Us!****/\n");
			bzero(file_name, 64);
			bzero(write_buf,100);
			struct RFID_table *new = calloc(1,sizeof(struct RFID_table));
			new->id_num = id_num;
			sprintf(file_name,"%u.jpg",id_num);
			strcpy(new->file_name,file_name);
			sprintf(write_buf,"%u %u.jpg\n",id_num,id_num);
			list_add_tail(&new->list,&head->list);
			fp = fopen("./RFID_link.txt", "a+");
			if(fp == NULL)
			{
				perror("open RFID_link.txt \n");
				return;
			}
			fwrite(write_buf,strlen(write_buf),1,fp);
			
			fseek(fp,0,SEEK_SET);
			show_Txt_info(fp);
			fclose(fp);
			encode_jpeg(frame_buffer,file_name,IMAGEWIDTH,IMAGEHEIGHT); //RGB24 to Jpeg 
			printf("/***********ID Add Success,You can Add Another ID*********/\n");
			printf("/***************Change Mode Please Select: ***************/\n");
			printf("/***************select 2:DEL ID in SYS********************/\n");
			printf("/***************select 3:Back To Standard Mode************/\n");
			printf("/***************select 0:SYS EXIT*************************/\n");
		}
	}
}

void rfid_del_id(void)
{
	int i=0;
	unsigned int id_num=0;
	char write_buf[100];
	struct list_head *pos;
	struct list_head *n;
	struct RFID_table *p;
	FILE * fp;	

	id_num = Hex_Num(buf);
	lcd_init(0,0,Color_Black);
	if(list_empty(&head->list)==1)
	{
		printf("/*****Still have no Member in the list,No ID can be delete!****/\n");
		printf("please select working mode:\n");
		printf("select 1:Add ID in SYS\n");
		printf("select 3:Back To Standard Mode\n");
		printf("select 0:SYS EXIT\n");
	}
	else
	{
		list_for_each_safe(pos,n,&head->list)
		{
			p = list_entry(pos,struct RFID_table,list);
			
			if(p->id_num == id_num)	
			{
				remove(p->file_name);
				list_del(pos);
				n->prev->next = n->next->prev;
				
				i=0;
				break;
			}
			i++;
		}
		if(i != 0)
		{
			printf("/****************NO ID Match**************/\n");
		}
		if(i==0)
		{
			fp = fopen("./RFID_link.txt", "w+");
			if(fp == NULL)
			{
				perror("open RFID_link.txt \n");
				return;
			}
			list_for_each(pos, &head->list)
			{
				p = list_entry(pos,struct RFID_table,list);
				
				sprintf(write_buf,"%u %u.jpg\n",p->id_num,p->id_num);
				fwrite(write_buf,strlen(write_buf),1,fp);
			}
			fseek(fp,0,SEEK_SET);
			show_Txt_info(fp);
			printf("/************ID Match ,Cancel ID OK!************/\n");
						
			printf("/***************Change Mode Please Select: ****************/\n");
			printf("/***************select 2:DEL ID in SYS********************/\n");
			printf("/***************select 3:Back To Standard Mode*************/\n");
			printf("/***************select 0:SYS EXIT************************/\n");
		}
		
	}
}

void rfid_standard_mode(void)
{
	int x,y,j=0;
	unsigned int id_num=0;
	struct list_head *pos;
	struct RFID_table *p;
	int color = Color_Black;
	
	id_num = Hex_Num(buf);
	lcd_init(0,0,Color_Black);
	if(list_empty(&head->list)==1)
	{
		printf("/*****Still have no Member in the list,try add ID!****/\n");
		printf("please select working mode:\n");
		printf("select 1:Add ID in SYS\n");
		printf("select 2:DEL ID in SYS\n");
		printf("select 3:Back To Standard Mode\n");
		printf("select 0:SYS EXIT\n");
	}
	else
	{	
		for(y=0;y < LCD_HEIGHT; y++)
		{
			for (x=0;x < LCD_WIDTH; x++)
			{
				lcd_draw_point(x, y, color);				
			}
		}
		list_for_each(pos,&head->list)
		{
			p = list_entry(pos,struct RFID_table,list);
			if(p->id_num == id_num)	
			{
				printf("/************ID Match , and Show************/\n");
				lcd_draw_jpg(OFFSET_X,OFFSET_Y,p->file_name,NULL,0,0);
				j=0;
				break;
			}
			j++;
		}
		if(j != 0)
		{
			printf("/****************NO ID Match**************/\n");
		}
	}
}





 


