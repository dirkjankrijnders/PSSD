/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>

#include "lib/lcd.h"
#include "lib/keypad.h"
//#include "lib/i2cmaster.h"

//#include "../firmware/PSSD.h"

#define addPSSD  0x20

char prog[] = {'-','/','|','\\'};

int main(void)
{
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	lcd_puts("Hello world\0");
	
	keypad_init();
	
	i2c_init();
	
	char key = '\0';
	unsigned char ret = 0;
	unsigned char board;
	uint8_t col = 1;
    for(;;){
        /* insert your main loop code here */
		lcd_goto(1,0);
		lcd_puts("Scan I2C");
		lcd_putch(prog[ret]);
		i2c_start_wait(addPSSD+I2C_WRITE);
		//if (i2c_start(addPSSD+I2C_WRITE)) {
			
			i2c_write(FW_VERSION_REG);
			i2c_rep_start(addPSSD+I2C_READ);       // set device address and read mode
			
			ret = i2c_readNak();                    // read one byte from EEPROM
			i2c_stop();
			lcd_goto(0,1);
			board = ret;
			switch (ret) {
				case PSSD:
					lcd_puts("PSSD");
					break;
				case CSMD:
					lcd_puts("C Rail Servo mount");
					break;
					
				default:
					lcd_puts("Unknown board: ");
					lcd_putch(board + 48);
					board = 0;
					break;
			}
		//}
		while (board) {
			for (col = 1; col < 5; col++) {
				lcd_goto(1,0);
				key = keypad_read_key(col);
				if (key) {
					lcd_putch(key+48);
					switch (col) {
						case 1:
							//sw
							break;
							
						default:
							break;
					}
				}
				//	lcd_putch(col+48);
				/*				col++;
				 if (col == 5) {
				 lcd_goto(0,0);
				 col	= 1;
				 }*/
			}
		}
		ret++;
		if (ret == 4) ret = 0;
	}
	return 0;   /* never reached */
}
