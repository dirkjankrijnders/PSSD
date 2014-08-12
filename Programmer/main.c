/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#include "lib/lcd.h"
#include "lib/keypad.h"
#include "lib/i2cmaster.h"

#include "../firmware/PSSD.h"

#define PORT_I2C PORTB
#define DDR_I2C DDRB
#define PIN_I2C PINB
#define P_I2C PB1

enum state_t {
	SCAN = 0,
	GETINFO,
	SETADDA,
	SETSHORTA,
	SETLONGA,
	SETADDB,
	SETSHORTB,
	SETLONGB,
	SAVEPARAMA,
	SAVEPARAMB
} state;

typedef struct {
	uint8_t address;
	uint16_t shorts;
	uint16_t longs;
	uint8_t position;
}info_t ;

#define addPSSD  0x20

//char prog[] = {'-','/','|','\\'};
uint8_t icbuf[2];
info_t info[2];
uint8_t digit;
uint16_t val;
uint8_t sel[] = {0, 5, 10};

uint16_t get_val() {
	switch (state) {
		case SETADDA:
			return info[0].address;
		case SETADDB:
			return info[1].address;
		case SETSHORTA:
			return info[0].shorts;
		case SETSHORTB:
			return info[1].shorts;
		case SETLONGA:
			return info[0].longs;
		case SETLONGB:
			return info[1].longs;
		default:
			break;
	}
	return 0;
}

uint8_t read_i2c_reg(uint8_t add, uint8_t reg) {
	uint8_t ret;
	PORT_I2C |= (1 << P_I2C);
	_delay_us(10);
	i2c_start_wait(add+I2C_WRITE);
	i2c_write(reg);
	i2c_rep_start(add+I2C_READ);       // set device address and read mode
	ret = i2c_readNak();                    // read one byte from EEPROM
	i2c_stop();
	PORT_I2C &= ~(1 << P_I2C);
	return ret;
}

uint16_t read_i2c_reg16(uint8_t add, uint8_t reg) {
	icbuf[0] = read_i2c_reg(add, reg);
	icbuf[1] = read_i2c_reg(add, reg+1);
	return *(uint16_t*)icbuf;
}

void write_i2c_reg(uint8_t add, uint8_t reg, uint8_t value) {
	PORT_I2C |= (1 << P_I2C);
	_delay_us(10);
	i2c_start_wait(add+I2C_WRITE);
	i2c_write(reg);
	i2c_write(value);
	i2c_stop();
	PORT_I2C &= ~(1 << P_I2C);
}

void write_i2c_reg16(uint8_t add, uint8_t reg, uint16_t value) {
	write_i2c_reg(add, reg, value & 0xFF);
	write_i2c_reg(add, reg+1, value >> 8);
};


void goto_sel(uint8_t off) {
	uint8_t mode = state - 2;
	uint8_t line = mode / 3;
	mode  = mode - (line * 3);
	lcd_goto(sel[mode]+off, line);
}

void update_field() {
	goto_sel(1);
	char buf[4];
	sprintf(buf, "%4u", val);
	lcd_puts(buf);
}

void proces_digit(uint8_t value) {
	if (digit == 0) {
		val = 0;
		digit = 1;
	};
	
	val = val*10 + value;
	update_field();
}

void save() {
	switch (state) {
		case SETADDA:
			write_i2c_reg(addPSSD, ADD_A_REG, val);
			info[0].address = val;
			break;
		case SETADDB:
			write_i2c_reg(addPSSD, ADD_B_REG, val);
			info[1].address = val;
			break;
		case SETSHORTA:
			write_i2c_reg16(addPSSD, SHORT_A_REG_L, val);
			info[0].shorts = val;
			break;
		case SETSHORTB:
			write_i2c_reg16(addPSSD, SHORT_B_REG_L, val);
			info[1].shorts = val;
			break;
		case SETLONGA:
			write_i2c_reg16(addPSSD, LONG_A_REG_L, val);
			info[0].longs = val;
			break;
		case SETLONGB:
			write_i2c_reg16(addPSSD, LONG_B_REG_L, val);
			info[1].longs = val;
			break;
		default:
			break;
	}
}


void show_settings(uint8_t line, info_t i) {
	char buf[16];
	char p = (i.position ? '/' : '|');
	lcd_goto(1, line);
	sprintf(buf, "%4u %4u %4u%c", i.address, i.shorts, i.longs, p);
	lcd_puts(buf);
}

int main(void)
{
	DDR_I2C |= (1 << P_I2C);
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	//lcd_puts("Hello world\0");
	
	keypad_init();
	
	i2c_init();
	
	char key = '\0';
//	unsigned char ret = 0;
	unsigned char board;
	state = SCAN;
	uint8_t col = 1;
    for(;;){
        /* insert your main loop code here */
		switch (state) {
			case SCAN:
				PORT_I2C |= (1 << P_I2C);
				lcd_goto(0,0);
				lcd_puts("Scan I2C");
				//				lcd_putch(prog[ret]);
				i2c_start_wait(addPSSD+I2C_WRITE);
				//if (i2c_start(addPSSD+I2C_WRITE)) {
				
				i2c_write(FW_VERSION_REG);
				i2c_rep_start(addPSSD+I2C_READ);       // set device address and read mode
				
				board = i2c_readNak();                    // read one byte from EEPROM
				i2c_stop();
				PORT_I2C &= ~(1 << P_I2C);
				lcd_goto(0,1);
				switch (board) {
					case PSSD:
						lcd_puts("PSSD");
						state = GETINFO;
						break;
					case CSMD:
						state = GETINFO;
						lcd_puts("C Rail Servo mount");
						break;
						
					default:
						lcd_puts("Unknown board: ");
						lcd_putch(board + 48);
						board = 0;
						break;
				}
				//state++;
				break;
			case GETINFO:
				info[0].address = read_i2c_reg(addPSSD, ADD_A_REG);
				info[0].longs = read_i2c_reg16(addPSSD, LONG_A_REG_L);
				info[0].shorts = read_i2c_reg16(addPSSD, SHORT_A_REG_L);
				info[0].position = 1;//read_i2c_reg(addPSSD, POSITION_A_REG);
				info[1].address = read_i2c_reg(addPSSD, ADD_B_REG);
				info[1].longs = read_i2c_reg16(addPSSD, LONG_B_REG_L);
				info[1].shorts = read_i2c_reg16(addPSSD, SHORT_B_REG_L);
				info[1].position = 1;//read_i2c_reg(addPSSD, POSITION_B_REG);
				show_settings(0, info[0]);
				show_settings(1, info[1]);
				state++;
				val = get_val();
				lcd_goto(0,0);
				lcd_putch('>');
				break;
			default:
				for (col = 0; col < 4; col++) {
					lcd_goto(0,0);
					key = keypad_read_key(col+1);
					if (key) {
						key += col * 4;
						switch (key) {
							case 1:
								proces_digit(1);
								break;
							case 2:
								proces_digit(4);
								break;
							case 3:
								proces_digit(7);
								break;
							case 5:
								proces_digit(2);
								break;
							case 6:
								proces_digit(5);
								break;
							case 7:
								proces_digit(8);
								break;
							case 8:
								proces_digit(0);
								break;
							case 9:
								proces_digit(3);
								break;
							case 10:
								proces_digit(6);
								break;
							case 11:
								proces_digit(9);
								break;
							case 4:
								val -= 10;
								update_field();
								save();
								digit = 0;
								break;
							case 12:
								val += 10;
								update_field();
								save();
								digit = 0;
								break;
							case 13:
								goto_sel(0);
								lcd_putch(' ');
								state++;
								if (state > SETLONGB) state = SETADDA;
								val = get_val();
								digit = 0;
								goto_sel(0);
								lcd_putch('>');
								break;
							case 14:
								save();
							case 15:
								if (info[0].position == 0) {
									info[0].position =1;
								} else {
									info[0].position = 0;
								}
								write_i2c_reg(addPSSD, POSITION_A_REG, info[0].position);
								if (info[1].position == 0) {
									info[1].position =1;
								} else {
									info[1].position = 0;
								}
								write_i2c_reg(addPSSD, POSITION_B_REG, info[1].position);
								show_settings(0, info[0]);
								show_settings(1, info[1]);
								break;
							case 16:
								state = SCAN;
							default:
								break;
						}
					}
				}
				break;
		}
	}
	return 0;   /* never reached */
}
