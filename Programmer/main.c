/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>

#include "lcd.h"

int main(void)
{
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	lcd_puts("Hello world\0");
    for(;;){
        /* insert your main loop code here */
    }
    return 0;   /* never reached */
}
