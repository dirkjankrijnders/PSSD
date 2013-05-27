/*#define N 64
#define TOP 1/(N*50/F_CPU)-1
#define TOP 4999
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h> 
#include "mm_module.h"

#define DEBOUNCE_TIME 25

enum state{
	NORMAL = 0,
	SET_A,
	SET_B,
	SET_ADDRESS
} state;

enum adjState {
	short_ = 0,
	long_
} adjState;

uint16_t EEMEM eShortA = 30;
uint16_t EEMEM eShortB = 30;
uint16_t EEMEM eLongA = 50;
uint16_t EEMEM eLongB = 50;
uint8_t  EEMEM eAddA = 0xC0;
uint8_t  EEMEM eAddB = 0xC0;
uint8_t  EEMEM ePortA = 0;
uint8_t  EEMEM ePortB = 2;

uint16_t EEMEM eLastA = 30;
uint16_t EEMEM eLastB = 30;

int main() {
    /* Declare variables for addres and data */
    uint8_t                                 MM_Address;
    uint8_t                                 MM_Data;
	
    /* Initialize the MM module */
    MM_Module_Init();

	// Setup timer1 in PWM mode 7. We need a frequency around 40 Hz and the uC runs at 10 MHz => factor 250.000
	// Count to 0x00FF => Prescaler /1024 => 38 Hz
	// Count to 0x01FF => Prescaler /256  => 76 Hz
	// Count to 0x03FF => Prescaler /256  => 38 Hz
	// Lowest prescaler (/256) gives best resolution
	TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11|(1<<WGM10));        // Inverted PWM
	TCCR1B=(1<<CS12)|(1<<WGM12); // PRESCALER=/256 MODE 7(FAST PWM, TOP=0x03FF)
	
	OCR1A = eeprom_read_word(&eLastA);
	OCR1B = eeprom_read_word(&eLastB);

	//Make sure OC1n are output
	DDRD = 0;
	DDRB|=(1<<PB4)|(1<<PB3); //PWM Pins as Out(1<<PD3)|
	DDRD|=(1<<PD0)|(1<<PD6); // GND Fet connection & LED
	
	// Set pull-up resistors for buttons
	PORTD = 0b00111010;

	uint8_t AddA = eeprom_read_byte(&eAddA);
	uint8_t AddB = eeprom_read_byte(&eAddB);
	uint8_t PortA = eeprom_read_byte(&ePortA);
	uint8_t PortB = eeprom_read_byte(&ePortB);
	uint16_t shortA = eeprom_read_word(&eShortA);
	uint16_t longA  = eeprom_read_word(&eLongA);
	uint16_t shortB = eeprom_read_word(&eShortB);
	uint16_t longB  = eeprom_read_word(&eLongB);
	
	sei();

	PORTD = PORTD ^ (1<<PD6);
	while(1) {
        /* New data received? */
        if ((MM_CheckForNewInfo(&MM_Address, &MM_Data) == MM_NEW_INFO) & (state == NORMAL))
        {
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
							PORTD|=(1<<PD0);
						} else if (MM_Data == PortA + 1) {
							OCR1A = longA;
							eeprom_write_word(&eLastA, OCR1A);
							PORTD|=(1<<PD0);
						}
						
						/* Restore MM_Data for next servo */
						MM_Data += 8;
					}
				}
			}
			if (MM_Address == AddB)
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
						if (MM_Data == PortB) {
							OCR1B = shortB;
							eeprom_write_word(&eLastB, OCR1B);
							PORTD|=(1<<PD0);
						} else if (MM_Data == PortB + 1) {
							OCR1B = longB;
							eeprom_write_word(&eLastB, OCR1B);
							PORTD|=(1<<PD0);
						}
					}
				}
			}
		} // New data
		
		/* B1/B2/B3/B4 as manual override */
		if (~PIND & (1<<PD3)){ // B1
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD3)){
				if (state == NORMAL)
					state = SET_A;
				
				shortA = eeprom_read_word(&eShortA);
				longA  = eeprom_read_word(&eLongA);
				shortB = eeprom_read_word(&eShortB);
				longB  = eeprom_read_word(&eLongB);
				OCR1A = shortA;
				OCR1B = shortB;
				adjState = short_;
			} else if (state == SET_A) {
				state = SET_B;
			} else if (state == SET_B) {
				// Save values
				eeprom_write_word(&eShortA, ShortA);
				eeprom_write_word(&eLongA, LongA);
				eeprom_write_word(&eShortB, ShortB);
				eeprom_write_word(&eLongB, LongB);
				state = NORMAL;
			}
		} 
		if (~PIND & (1<<PD4)){
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD4)){
				if (state == SET_A) {
					if (adjState == short_) {
						shortA++;
						OCR1A = shortA;
					} else {
						longA++;
						OCR1A = longA;
					}
				} else if (state == SET_B)  {
					if (adjState == short_) {
						shortB++;
						OCR1B = shortB;
					} else {
						longB++;
						OCR1B = longB;
					}
				}
				PORTD|=(1<<PD0);
			}
		}
		if (~PIND & (1<<PD5)){
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD5)){
				if (state == SET_A) {
					if (adjState == short_) {
						shortA--;
						OCR1A = shortA;
					} else {
						longA--;
						OCR1A = longA;
					}
				} else if (state == SET_B)  {
					if (adjState == short_) {
						shortB--;
						OCR1B = shortB;
					} else {
						longB--;
						OCR1B = longB;
					}
				}
				PORTD|=(1<<PD0);
			}
		}
		if (~PIND & (1<<PD1)){
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD1)){
				if (state == SET_A) {
					if (adjState == short_) {
						OCR1A = longA;
						adjState = long_;
					} else if (adjState == long_) {
						OCR1A = shortA;
						adjState = short_;
					}
				} else if (state == SET_B) {
					if (adjState == short_) {
						OCR1B = longB;
						adjState = long_;
					} else if (adjState == long_) {
						OCR1B = shortB;
						adjState = short_;
					}
				} else if (state == NORMAL) {
					state = SET_ADDRESS;
				} else if (state == SET_ADDRESS) {
					// TODO: Save address
					state = NORMAL;
				}
			}
		}
	} // While
}
