/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>

#include "lib/lcd.h"
#include "lib/keypad.h"
#include "lib/i2cmaster.h"

#include "../firmware/PSSD.h"

#define addPSSD  0x08

int main(void)
{
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	lcd_puts("Hello world\0");
	
	keypad_init();
	
	i2c_init();
	
	char key = '\0';
	unsigned char ret;
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
		if (i2c_start(addPSSD+I2C_WRITE)) {
		
			i2c_write(0x01);
			i2c_rep_start(addPSSD+I2C_READ);       // set device address and read mode
		
			ret = i2c_readNak();                    // read one byte from EEPROM
			i2c_stop();
			lcd_goto(1,0);
			switch (ret) {
				case PSSD:
					lcd_puts("PSSD");
					break;
				case CSMD:
					lcd_puts("C Rail Servo mount");
					break;
					
				default:
					break;
			}
		}
    }
    return 0;   /* never reached */
}
