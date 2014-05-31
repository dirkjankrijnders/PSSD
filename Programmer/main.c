/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>

#include "lib/lcd.h"
#include "lib/keypad.h"

int main(void)
{
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	lcd_puts("Hello world\0");
	
	keypad_init();
	
	char key = '\0';
	uint8_t col = 1;
    for(;;){
        /* insert your main loop code here */
		key = keypad_read_key(col);
		if (key) {
			lcd_putch(key+48);
		}
		//lcd_putch(col+48);
		col++;
		if (col == 5) {
			lcd_goto(0,0);
			col = 1;
		}
    }
    return 0;   /* never reached */
}
