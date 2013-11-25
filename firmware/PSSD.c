/*#define N 64
#define TOP 1/(N*50/F_CPU)-1
#define TOP 4999
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h> 
#include "usitwir/src/slave.h"
#include "mm_module.h"

#ifndef ADD_A
#define ADD_A 0xB0
#endif

#ifndef ADD_B
#define ADD_B 0x20
#endif

#ifndef PORT_A
#define PORT_A 0
#endif

#ifndef PORT_B
#define PORT_B 2
#endif

#define TYPE_REG 0x01
#define HW_VERSION_REG 0x02
#define FW_VERSION_REG 0x03
#define NAME_REG 0x04

#define ADD_A_REG 0x10
#define ADD_B_REG 0x20

#define SHORT_A_REG_L 0x11
#define SHORT_A_REG_H 0x12
#define LONG_A_REG_L 0x13
#define LONG_A_REG_H 0x14

#define SHORT_B_REG_L 0x21
#define SHORT_B_REG_H 0x22
#define LONG_B_REG_L 0x23
#define LONG_B_REG_H 0x24

#define POSITION_A_REG 0xE0
#define POSITION_B_REG 0xF0

#define NULL_REGISTER 0xFF

enum state{
	NORMAL = 0,
	I2C
} state;

uint8_t usitwi_address = 0x10;
uint8_t currentRegister = NULL_REGISTER;

uint16_t EEMEM eShortA = 1250;
uint16_t EEMEM eShortB = 1250;
uint16_t EEMEM eLongA = 1500;
uint16_t EEMEM eLongB = 1500;
uint8_t  EEMEM eAddA = ADD_A ;
uint8_t  EEMEM eAddB = ADD_B ;
uint8_t  EEMEM ePortA = PORT_A ;
uint8_t  EEMEM ePortB = PORT_B ;

uint16_t EEMEM eLastA = 1250;
uint16_t EEMEM eLastB = 1250;

uint8_t AddA    = 0;
uint8_t PortA   = 0;
uint16_t shortA = 0;
uint16_t longA  = 0;

uint8_t AddB    = 0;
uint8_t PortB   = 0;
uint16_t shortB = 0;
uint16_t longB  = 0;


int main()  __attribute__ ((noreturn));

int main() {
    /* Declare variables for addres and data */
    uint8_t                                 MM_Address;
    uint8_t                                 MM_Data;
	
    /* Initialize the MM module */
    MM_Module_Init();

    /* Initialize the I2C module */
    usitwi_init();
	
	// Setup timer1 in PWM mode 7. We need a frequency around 40 Hz and the uC runs at 10 MHz => factor 250.000
	// Count to 0x00FF => Prescaler /1024 => 38 Hz
	// Count to 0x01FF => Prescaler /256  => 76 Hz
	// Count to 0x03FF => Prescaler /256  => 38 Hz
	// Count to 250000 => Prescaler /8    => 50 Hz
	// Lowest prescaler (/256) gives best resolution
	// TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);//|(1<<WGM10);        // Inverted PWM
	// TCCR1B=(1<<CS11)|(1<<WGM12)|(1<<WGM13); // PRESCALER=/8 MODE 14(FAST PWM, TOP=ICR1)
	// ICR1 = 25000;
	// Measured resolution: 22 uS
	TCCR1 |= (1<<CS13)|(1<<CS11)|(1<<CS10); // Prescaler to /1024 => f_TCK1 =~ 8kHz (7812.5 Hz)
	TCCR1 |= (1<<PWM1A)|(1<<COM1A1)|(0<<COM1A0); // Set on $00, clear on match
	GTCCR |= (1<<PWM1B)|(1<<COM1B1)|(0<<COM1B0); // Set on $00, clear on match
	

	OCR1A = 0x1F; //eeprom_read_word(&eLastA);
	OCR1B = 0x2F; //eeprom_read_word(&eLastB);
	OCR1C = 0x3F;

	//Make sure OC1n are output
//RD = 0;
//DRB|=(1<<PB4)|(1<<PB3); //PWM Pins as Out(1<<PD3)|
//DRD|=(1<<PD0)|(1<<PD6); // GND Fet connection & LED
    DDRB |= (1 << PB1) | (1 << PB4);
//    PORTB = 0x00;

	AddA = eeprom_read_byte(&eAddA);
	PortA = eeprom_read_byte(&ePortA);
	shortA = eeprom_read_word(&eShortA);
	longA = eeprom_read_word(&eLongA);
	
    AddB = eeprom_read_byte(&eAddB);
	PortB = eeprom_read_byte(&ePortB);
	shortB = eeprom_read_word(&eShortB);
	longB = eeprom_read_word(&eLongB);

	sei();

//	PORTB = PORTB^ (1<<PB3);
	for(;;) {
        /* New data received? */
        if (MM_CheckForNewInfo(&MM_Address, &MM_Data) == MM_NEW_INFO) {
			if (state == NORMAL) {
        	
        		/* Address valid, check if bit 4 of MC145027 data is high */
        		if (MM_Address == AddA)
        		{
					/* If a 'normal' function bit is present (turnout) process it */
        			if (!(MM_Data & 0x10))
        			{
        				/* Bit 4 high for activating a turnout?? */
        				if (MM_Data & 0x08)
        				{
        					/* yes, process it, first remove bit 4 */
        					MM_Data -= 8;
        	   	       	   		
        					/* Now depedning on value, set a output */
							if (MM_Data == PortA) {
								OCR1A = shortA;
								eeprom_write_word(&eLastA, OCR1A);
								PORTB|=(1<<PB3);
							} else if (MM_Data == PortA + 1) {
								OCR1A = longA;
								eeprom_write_word(&eLastA, OCR1A);
								PORTB|=(1<<PB3);
							}
							
							/* Restore MM_Data for next servo */
							MM_Data += 8;
						}
					}
				}
			}
		} // New data
	} // While
}

void usitwi_onStart(uint8_t read) {
	if (!read) {
		currentRegister = NULL_REGISTER;
	}
}

void usitwi_onStop() {
	currentRegister = NULL_REGISTER;
}

uint8_t usitwi_onRead() {
	switch(currentRegister) {
		case TYPE_REG:
			return 0x01; // Original PSSD Servo/Switch Decoder
			
		case HW_VERSION_REG:
			return 0x01; // The 5x5 cm print, rev. 0
		case FW_VERSION_REG:
			return 0x02; // First I2C version
		case NAME_REG:
			return 0xFF;
		case ADD_A_REG:
			return eeprom_read_byte(&eAddA);
		case ADD_B_REG:
			return eeprom_read_byte(&eAddB);
		case SHORT_A_REG_L:
			return (eeprom_read_word(&eShortA)) & 0xFF;
		case SHORT_A_REG_H:
			return (eeprom_read_word(&eShortA) >> 8) & 0xFF;
		case LONG_A_REG_L:
			return (eeprom_read_word(&eLongA)) & 0xFF;
		case LONG_A_REG_H:
			return (eeprom_read_word(&eLongA) >> 8) & 0xFF;
		case SHORT_B_REG_L:
			return (eeprom_read_word(&eShortB)) & 0xFF;
		case SHORT_B_REG_H:
			return (eeprom_read_word(&eShortB) >> 8) & 0xFF;
		case LONG_B_REG_L:
			return (eeprom_read_word(&eLongB)) & 0xFF;
		case LONG_B_REG_H:
			return (eeprom_read_word(&eLongB) >> 8) & 0xFF;
		case POSITION_A_REG:
			if (OCR1A == shortA)
				return 0x00;
			else if (OCR1A == longA)
				return 0x01;
			else
				return 0x02; // Should never happen
		case POSITION_B_REG:
			if (OCR1B == shortB)
				return 0x00;
			else if (OCR1B == longB)
				return 0x01;
			else
				return 0x02; // Should never happen
		default:
			break;
	}
	return 0xFF;
}

void usitwi_onWrite(uint8_t value) {
	if (currentRegister == NULL_REGISTER) {
		currentRegister = value;
	} else {
		switch(currentRegister) {
			case ADD_A_REG:
				AddA = value;
				eeprom_write_byte(&eAddA, AddA);
				break;
			case ADD_B_REG:
				AddB = value;
				eeprom_write_byte(&eAddB, AddB);
				break;
			case SHORT_A_REG_L:
				shortA = value;
				break;
			case SHORT_A_REG_H:
				shortA = shortA + (value << 8);
				eeprom_write_word(&eShortA, shortA);
				break;
			case SHORT_B_REG_L:
				shortB = value;
				break;
			case SHORT_B_REG_H:
				shortB = shortB + (value << 8);
				eeprom_write_word(&eShortB, shortB);
				break;
			case LONG_A_REG_L:
				longA = value;
				break;
			case LONG_A_REG_H:
				longA = longA + (value << 8);
				eeprom_write_word(&eLongA, longA);
				break;
			case LONG_B_REG_L:
				longB = value;
				break;
			case LONG_B_REG_H:
				longB = longB + (value << 8);
				eeprom_write_word(&eLongB, longB);
				break;
			case POSITION_A_REG:
				if (value == 0x00) {
					OCR1A = shortA;
					eeprom_write_word(&eLastA, OCR1A);
				} else if (value == 0x01) {
					OCR1A = longA;
					eeprom_write_word(&eLastA, OCR1A);
				}
				break;
			case POSITION_B_REG:
				if (value == 0x00) {
					OCR1B = shortB;
					eeprom_write_word(&eLastB, OCR1A);
				} else if (value == 0x01) {
					OCR1B = longB;
					eeprom_write_word(&eLastB, OCR1A);
				}
				break;

			default:
				break;
		}
	}
}