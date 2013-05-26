/*#define N 64
#define TOP 1/(N*50/F_CPU)-1
#define TOP 4999
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h> 
#include "mm_module.h"

uint16_t EEMEM eShortA = 30;
uint16_t EEMEM eShortB = 30;
uint16_t EEMEM eLongA = 50;
uint16_t EEMEM eLongB = 50;
uint8_t  EEMEM eAddA = 0xC0;
uint8_t  EEMEM eAddB = 0xC0;
uint8_t  EEMEM ePortA = 0;
uint8_t  EEMEM ePortB = 2;

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
	
	OCR1A = 0xFF;
	OCR1B = 0x1FF;

	//Make sure OC1n are output
	DDRB|=(1<<PB4)|(1<<PB3); //PWM Pins as Out(1<<PD3)|
	DDRD|=(1<<PD0)|(1<<PD6); // GND Fet connection & LED
		
	
//	uint8_t thrown[2] = {0, 0};
	uint8_t AddA = eeprom_read_byte(&eAddA);
	uint8_t AddB = eeprom_read_byte(&eAddB);
	uint8_t PortA = eeprom_read_byte(&ePortA);
	uint8_t PortB = eeprom_read_byte(&ePortB);
	
	sei();

	PORTD = PORTD ^ (1<<PD6);
	while(1) {
        /* New data received? */
        if (MM_CheckForNewInfo(&MM_Address, &MM_Data) == MM_NEW_INFO)
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
							OCR1A = eeprom_read_word(&eShortA);
							PORTD|=(1<<PD0);
						} else if (MM_Data == PortA + 1) {
							OCR1A = eeprom_read_word(&eLongA);
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
					PORTD = PORTD ^ (1<<PD6);
	       			if (MM_Data & 0x08)
	       			{
    	   				/* yes, process it, first remove bit 4 */
	       				MM_Data -= 8;
            	  	   		
	       				/* Now depedning on value, set a output */
						if (MM_Data == PortB) {
							OCR1B = eeprom_read_word(&eShortB);
							PORTD|=(1<<PD0);
						} else if (MM_Data == PortB + 1) {
							OCR1B = eeprom_read_word(&eLongB);
							PORTD|=(1<<PD0);
						}
					}
				}
			}
		}
	}
}
