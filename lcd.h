#ifndef __LCD_H__
#define __LCD_H__

#define  LCD_WIDTH  		1024
#define  LCD_HEIGHT 		600
#define  IMAGEWIDTH   	 	640  
#define  IMAGEHEIGHT   		480

#define  OFFSET_X			192
#define  OFFSET_Y			60
#define  Color_White 		0xffffff
#define  Color_Black		0

#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)
#define EN_LCD_SHOW_JPG		1

extern unsigned char frame_buffer[IMAGEHEIGHT*IMAGEWIDTH*3];

#endif
