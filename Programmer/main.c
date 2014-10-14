/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

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
	SETPORTA,
	SETSPEEDA,
	SETADDB,
	SETSHORTB,
	SETLONGB,
	SETPORTB,
	SETSPEEDB,
	SAVEPARAMA,
	SAVEPARAMB
} state;

typedef struct {
	uint8_t address;
	uint8_t port;
	uint16_t shorts;
	uint16_t longs;
	uint8_t position;
	uint8_t	speed;
}info_t ;

#define addPSSD  0x20

//char prog[] = {'-','/','|','\\'};
uint8_t icbuf[2];
info_t info[2];
uint8_t digit;
uint16_t val;
uint8_t sel[] = {0, 5, 10, 0, 5, 10};
uint8_t no_output = 0;
uint8_t current_output = 0;

uint8_t dec_address_loopup(uint8_t trin_address);
uint8_t trin_address_lookup(uint8_t dec_address);

uint16_t get_val() {
	switch (state) {
		case SETADDA:
			return info[0].address;
		case SETADDB:
			return info[1].address;
		case SETPORTA:
			return info[0].port;
		case SETPORTB:
			return info[1].port;
		case SETSHORTA:
			return info[0].shorts;
		case SETSHORTB:
			return info[1].shorts;
		case SETLONGA:
			return info[0].longs;
		case SETLONGB:
			return info[1].longs;
		case SETSPEEDA:
			return info[0].speed;
		case SETSPEEDB:
			return info[1].speed;
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
	uint8_t line = mode / 5;
	mode  = mode - (line * 5);
	if (mode > 2)
		line = 1;
	else
		line = 0;
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
			write_i2c_reg(addPSSD, ADD_A_REG, trin_address_lookup(val));
			info[0].address = val;
			break;
		case SETADDB:
			write_i2c_reg(addPSSD, ADD_B_REG, trin_address_lookup(val));
			info[1].address = val;
			break;
		case SETPORTA:
			write_i2c_reg(addPSSD, PORT_A_REG, val);
			info[0].port = val;
			break;
		case SETPORTB:
			write_i2c_reg(addPSSD, PORT_B_REG, val);
			info[1].port = val;
			break;
		case SETSPEEDA:
			write_i2c_reg(addPSSD, SPEED_A_REG, val);
			info[0].speed = val;
			break;
		case SETSPEEDB:
			write_i2c_reg(addPSSD, SPEED_B_REG, val);
			info[1].speed = val;
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


void show_settings() { //uint8_t line, info_t i) {
	uint8_t output = state / SETADDB;
	info_t i = info[output];
	char buf[16];
	char p = (i.position ? '/' : '|');
	lcd_goto(1, 0);
	sprintf(buf, "%4u %4u %4u%c", i.address, i.shorts, i.longs, p);
	lcd_puts(buf);
	lcd_goto(0, 1);
	sprintf(buf, " %4u %4u      ", i.port, i.speed);
	lcd_puts(buf);
}

int main(void)
{
	DDR_I2C |= (1 << P_I2C);
	/* insert your hardware initialization here */
	lcd_init();//(LCD_DISP_ON_CURSOR_BLINK);
	//lcd_puts("Hello world\0");
	lcd_puts("PSSD Programmer");
	lcd_goto(0, 1);
	lcd_puts("v 0.1.0");
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
				board = 0;
				lcd_goto(0,0);
				lcd_puts("Scan I2C        ");
				do {
					board = read_i2c_reg(addPSSD, FW_VERSION_REG);
					_delay_us(10);
				} while (board == 0);
/*				PORT_I2C |= (1 << P_I2C);
				lcd_goto(0,0);
				//				lcd_putch(prog[ret]);
				i2c_start_wait(addPSSD+I2C_WRITE);
				//if (i2c_start(addPSSD+I2C_WRITE)) {
				
				i2c_write(FW_VERSION_REG);
				i2c_rep_start(addPSSD+I2C_READ);       // set device address and read mode
				
				board = i2c_readNak();                    // read one byte from EEPROM
				i2c_stop();
				PORT_I2C &= ~(1 << P_I2C);*/
				lcd_goto(0,1);
				switch (board) {
					case PSSD:
						lcd_puts("PSSD");
						state = GETINFO;
						no_output = 2;
						break;
					case CSMD:
						state = GETINFO;
						lcd_puts("C Rail Servo mount");
						no_output = 2;
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
				info[0].address = dec_address_loopup(read_i2c_reg(addPSSD, ADD_A_REG));
				info[0].longs = read_i2c_reg16(addPSSD, LONG_A_REG_L);
				info[0].shorts = read_i2c_reg16(addPSSD, SHORT_A_REG_L);
				info[0].position = 1;//read_i2c_reg(addPSSD, POSITION_A_REG);
				info[1].address = dec_address_loopup(read_i2c_reg(addPSSD, ADD_B_REG));
				info[1].longs = read_i2c_reg16(addPSSD, LONG_B_REG_L);
				info[1].shorts = read_i2c_reg16(addPSSD, SHORT_B_REG_L);
				info[1].position = 1;//read_i2c_reg(addPSSD, POSITION_B_REG);
				show_settings();
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
								if (state > SETSPEEDB) state = SETADDA;
								show_settings();
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

uint8_t dec_address_loopup(uint8_t trin_address) {
	uint8_t ad = 0;
	for (ad = 0; ad < 256; ad++) {
		if (trin_address_lookup(ad) == trin_address)
			return ad;
	}
}

uint8_t trin_address_lookup(uint8_t dec_address) {
	static const  uint8_t add_lookup[256] PROGMEM =
	{
		0xAA, 0xC0, 0x80, 0x30, 0xF0, 0xB0, 0x20, 0xE0, 0xA0, 0x0C, 0xCC, 0x8C, 0x3C, 0xFC, 0xBC,
		0x2C, 0xEC, 0xAC, 0x08, 0xC8, 0x88, 0x38, 0xF8, 0xB8, 0x28, 0xE8, 0xA8, 0x03, 0xC3, 0x83,
		0x33, 0xF3, 0xB3, 0x23, 0xE3, 0xA3, 0x0F, 0xCF, 0x8F, 0x3F, 0xFF, 0xBF, 0x2F, 0xEF, 0xAF,
		0x0B, 0xCB, 0x8B, 0x3B, 0xFB, 0xBB, 0x2B, 0xEB, 0xAB, 0x02, 0xC2, 0x82, 0x32, 0xF2, 0xB2,
		0x22, 0xE2, 0xA2, 0x0E, 0xCE, 0x8E, 0x3E, 0xFE, 0xBE, 0x2E, 0xEE, 0xAE, 0x0A, 0xCA, 0x8A,
		0x3A, 0xFA, 0xBA, 0x2A, 0xEA, 0x00, 0x40, 0x60, 0x97, 0x70, 0x48, 0x68, 0x58, 0x78, 0x44,
		0x64, 0x54, 0x74, 0x4C, 0x6C, 0x5C, 0x7C, 0x42, 0x62, 0x52, 0x72, 0x4A, 0x6A, 0x5A, 0x7A,
		0x46, 0x66, 0x56, 0x76, 0x4E, 0x6E, 0x5E, 0x7E, 0x41, 0x61, 0x51, 0x71, 0x49, 0x69, 0x59,
		0x79, 0x45, 0x65, 0x9F, 0x75, 0x4D, 0x6D, 0x5D, 0x7D, 0x43, 0x63, 0x53, 0x73, 0x4B, 0x6B,
		0x5B, 0x7B, 0x47, 0x67, 0x57, 0x77, 0x4F, 0x6F, 0x5F, 0x7F, 0x10, 0x18, 0x14, 0x1C, 0x12,
		0x1A, 0x16, 0x1E, 0x11, 0x19, 0x15, 0x1D, 0x13, 0x1B, 0x17, 0x1F, 0xD0, 0xD8, 0xD4, 0xDC,
		0xD2, 0xDA, 0xD6, 0xDE, 0xD1, 0xD9, 0xD5, 0xDD, 0xD3, 0xDB, 0xD7, 0xDF, 0x90, 0x98, 0x94,
		0x9C, 0x92, 0x9A, 0x96, 0x9E, 0x91, 0x99, 0x95, 0x9D, 0x93, 0x9B, 0x50, 0x55, 0x04, 0x06,
		0x05, 0x07, 0xC4, 0xC6, 0xC5, 0xC7, 0x84, 0x86, 0x85, 0x87, 0x34, 0x36, 0x35, 0x37, 0xF4,
		0xF6, 0xF5, 0xF7, 0xB4, 0xB6, 0xB5, 0xB7, 0x24, 0x26, 0x25, 0x27, 0xE4, 0xE6, 0xE5, 0xE7,
		0xA4, 0xA6, 0xA5, 0xA7, 0x01, 0xC1, 0x81, 0x31, 0xF1, 0xB1, 0x21, 0xE1, 0xA1, 0x0D, 0xCD,
		0x8D, 0x3D, 0xFD, 0xBD, 0x2D, 0xED, 0xAD, 0x09, 0xC9, 0x89, 0x39, 0xF9, 0xB9, 0x29, 0xE9, 0xA9};
	return pgm_read_byte(&(add_lookup[dec_address]));
};
