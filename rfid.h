#ifndef __RFID_H__
#define __RFID_H__

#define g_ID_IN		1
#define g_ID_OUT	2
#define g_SYS_Wait  3
#define g_SYS_EXIT	0
#define g_SYS_Stand 9

#define UART1_DEV	"/dev/ttySAC1"

extern struct RFID_table *head;
extern char buf[32];
extern volatile int uart_fd;
extern pthread_mutex_t wr_lock;
extern pthread_mutex_t rd_lock;


void uart2_init();
unsigned int Hex_Num(char *buffer);
void show_Txt_info(FILE *fp);
void rfid_info_init(void);
void rfid_add_id(void);
void rfid_del_id(void);
void rfid_standard_mode(void);
struct RFID_table *init_list(void);

extern void lcd_close(void);
extern int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path,char *pjpg_buf,unsigned int jpg_buf_size,unsigned int jpg_half) ; 
extern int encode_jpeg(char *lpbuf,char *file_name,int width,int height);


#endif
