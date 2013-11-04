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
#define ADJ_DELAY 50
#define MAX_CNT 5000
#define MIN_CNT 20

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


void pssd_blink(uint8_t times) {
	PORTD |= (1<<PD6);
	for (uint8_t i = 0; i < times ; i++){
		_delay_ms(100);
		PORTD = PORTD ^ (1<<PD6);
	}
	};

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

uint8_t setAddress = 1;

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

void main()  __attribute__ ((noreturn));

void main() {
    /* Declare variables for addres and data */
    uint8_t                                 MM_Address;
    uint8_t                                 MM_Data;
	
    /* Initialize the MM module */
    MM_Module_Init();

	// Setup timer1 in PWM mode 7. We need a frequency around 40 Hz and the uC runs at 10 MHz => factor 250.000
	// Count to 0x00FF => Prescaler /1024 => 38 Hz
	// Count to 0x01FF => Prescaler /256  => 76 Hz
	// Count to 0x03FF => Prescaler /256  => 38 Hz
	// Count to 250000 => Prescaler /8    => 50 Hz
	// Lowest prescaler (/256) gives best resolution
	TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);//|(1<<WGM10);        // Inverted PWM
	TCCR1B=(1<<CS11)|(1<<WGM12)|(1<<WGM13); // PRESCALER=/8 MODE 14(FAST PWM, TOP=ICR1)
	ICR1 = 25000;
	// Measured resolution: 22 uS
	
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
			} else if (state == SET_ADDRESS) {
				if (MM_Data & 0x08 & !(MM_Data & 0x10)){ // Valid turnout activation
					if (setAddress) // A
					{
						AddA  = MM_Address;
						PortA = MM_Data - 8;
						setAddress--;
						pssd_blink(2);
						_delay_ms(500);
						pssd_blink(1);
				} else { // B
						AddB  = MM_Address;
						PortB = MM_Data - 8;
						setAddress++;
						eeprom_write_byte(&eAddA, AddA);
						eeprom_write_byte(&eAddB, AddB);
						eeprom_write_byte(&ePortA, PortA);
						eeprom_write_byte(&ePortB, PortB);
						pssd_blink(2);
						_delay_ms(500);
						pssd_blink(2);
						state = NORMAL;
					}
				}
				
			}
		} // New data
		
		/* B1/B2/B3/B4 as manual override */
		if (~PIND & (1<<PD3)){ // B1
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD3)){
				if (state == NORMAL){
					state = SET_A;
					pssd_blink(2);

					
					shortA = eeprom_read_word(&eShortA);
					longA  = eeprom_read_word(&eLongA);
					shortB = eeprom_read_word(&eShortB);
					longB  = eeprom_read_word(&eLongB);
					OCR1A = shortA;
					OCR1B = shortB;
					adjState = short_;
					PORTD|=(1<<PD0);
				} else if (state == SET_A) {
					pssd_blink(3);
					state = SET_B;
				} else if (state == SET_B) {
					pssd_blink(4);
					// Save values
					eeprom_write_word(&eShortA, shortA);
					eeprom_write_word(&eLongA, longA);
					eeprom_write_word(&eShortB, shortB);
					eeprom_write_word(&eLongB, longB);
					state = NORMAL;
				}
			}
		} 
		if (~PIND & (1<<PD4)){
			_delay_ms(DEBOUNCE_TIME);
			if (~PIND & (1<<PD4)){
				if (state == SET_A) {
					if (adjState == short_) {
						shortA++;
						if (shortA > MAX_CNT)
							shortA = MAX_CNT;
						OCR1A = shortA;
						_delay_ms(ADJ_DELAY);
					} else {
						longA++;
						if (longA > MAX_CNT)
							longA = MAX_CNT;
						OCR1A = longA;
						_delay_ms(ADJ_DELAY);
					}
				} else if (state == SET_B)  {
					if (adjState == short_) {
						shortB++;
						if (shortB > MAX_CNT)
							shortB = MAX_CNT;
						OCR1B = shortB;
						_delay_ms(ADJ_DELAY);
					} else {
						longB++;
						if (longB > MAX_CNT)
							longB = MAX_CNT;
						OCR1B = longB;
						_delay_ms(ADJ_DELAY);
					}
				} else if(state == NORMAL) {
					if (OCR1A == longA)
						OCR1A = shortA;
					else
						OCR1A = longA;
					_delay_ms(1000);
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
						if (shortA < MIN_CNT)
							shortA = MIN_CNT;
						OCR1A = shortA;
					} else {
						longA--;
						if (longA < MIN_CNT)
							longA = MIN_CNT;
						OCR1A = longA;
					}
				} else if (state == SET_B)  {
					if (adjState == short_) {
						shortB--;
						if (shortB < MIN_CNT)
							shortB = MIN_CNT;
						OCR1B = shortB;
					} else {
						longB--;
						if (longB < MIN_CNT)
							longB = MIN_CNT;
						OCR1B = longB;
					}
				} else if(state == NORMAL) {
					if (OCR1B == longB)
						OCR1B = shortB;
					else
						OCR1B = longB;
					_delay_ms(1000);
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
					pssd_blink(2);
					state = SET_ADDRESS;
				} else if (state == SET_ADDRESS) {
					pssd_blink(2);
					state = NORMAL;
				}
				PORTD |= (1<<PD6);
				_delay_ms(1000);
				PORTD = PORTD ^ (1<<PD6);
			}
		}
	} // While
}
