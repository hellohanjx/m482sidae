#ifndef _12864_DRIVER_H_
#define _12864_DRIVER_H_

#include "M480.h"	

void _lcd_config(void);
void lcd_show_ercode(uint16_t x_p, uint16_t y_p, uint16_t xylong, uint8_t scale , uint8_t buf[][61] ) ;
void lcd_show_letter( char *letter, uint8_t x_p, uint8_t  y_p);
void lcd_clear(void);

void creat_ercode(uint16_t x, uint16_t y, uint8_t version);


#endif

