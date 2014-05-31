//
//  keypad.c
//  PSSDProgrammer
//
//  Created by Dirkjan Krijnders on 31/05/14.
//
//

#include <stdio.h>
#include <avr/io.h>

//uint8_t col = 0;
#define PORT_COL1 PORTB
#define PORT_COL2 PORTD
#define PORT_COL3 PORTD
#define PORT_COL4 PORTB
#define DDR_COL1 DDRB
#define DDR_COL2 DDRD
#define DDR_COL3 DDRD
#define DDR_COL4 DDRB
#define PORT_ROWS PORTD

#define PIN_COL1 PB0
#define PIN_COL2 PD7
#define PIN_COL3 PD6
#define PIN_COL4 PB7
#define PIN_ROWS PIND

void keypad_init(){
	// Set the rows with pullups
	PORT_ROWS = 0b00111100;
}

char keypad_read_key(uint8_t col) {
	char key = '\0';
	// Set the appropriate column as output;
	// Set it low
	// Read key
	// Set it as input again
	switch (col) {
		case 1:
			DDR_COL1 |= (1 << PIN_COL1);
			PORT_COL1 &= ~(1 << PIN_COL1);
			key = PIN_ROWS >> 2;
			DDR_COL1 &= ~(1 << PIN_COL1);
			lcd_puts("a\0");
			break;
		case 2:
			DDR_COL2 |= (1 << PIN_COL2);
			PORT_COL2 &= ~(1 << PIN_COL2);
			key = PIN_ROWS >> 2;
			DDR_COL2 &= ~(1 << PIN_COL2);
			break;
		case 3:
			DDR_COL3 |= (1 << PIN_COL3);
			PORT_COL3 &= ~(1 << PIN_COL3);
			key = PIN_ROWS >> 2;
			DDR_COL3 &= ~(1 << PIN_COL3);
			break;
		case 4:
			DDR_COL4 |= (1 << PIN_COL4);
			PORT_COL4 &= ~(1 << PIN_COL4);
			key = PIN_ROWS >> 2;
			DDR_COL4 &= ~(1 << PIN_COL4);
			break;
			
		default:
			break;
	}

	return key & 0x0F; // Only lower 4 bits
}
