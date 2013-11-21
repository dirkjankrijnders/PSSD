/*#define N 64
#define TOP 1/(N*50/F_CPU)-1
#define TOP 4999
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h> 
#include "usiTwi/usiTwiSlave.h"
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

enum state{
	NORMAL = 0,
	I2C
} state;

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

int main()  __attribute__ ((noreturn));

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
	// Count to 250000 => Prescaler /8    => 50 Hz
	// Lowest prescaler (/256) gives best resolution
	// TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);//|(1<<WGM10);        // Inverted PWM
	// TCCR1B=(1<<CS11)|(1<<WGM12)|(1<<WGM13); // PRESCALER=/8 MODE 14(FAST PWM, TOP=ICR1)
	// ICR1 = 25000;
	// Measured resolution: 22 uS
	TCCR1 |= (1<<CS13)|(0<<CS12)|(1<<CS11)|(1<<CS10); // Prescaler to /1024 => f_TCK1 =~ 8kHz (7812.5 Hz)
	TCCR1 |= (1<<PWM1A)|(1<<COM1A1)|(0<<COM1A0); // Set on $00, clear on match
	GTCCR |= (1<<PWM1B)|(1<<COM1B1)|(0<<COM1B0); // Set on $00, clear on match
	

	OCR1A = eeprom_read_word(&eLastA);
	OCR1B = eeprom_read_word(&eLastB);
	OCR1C = 0x3F;

	//Make sure OC1n are output
//RD = 0;
//DRB|=(1<<PB4)|(1<<PB3); //PWM Pins as Out(1<<PD3)|
//DRD|=(1<<PD0)|(1<<PD6); // GND Fet connection & LED
   DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB3) | (1 << PB4);
   PORTB = 0x00;

	uint8_t AddA = eeprom_read_byte(&eAddA);
	uint8_t PortA = eeprom_read_byte(&ePortA);
	uint16_t shortA = eeprom_read_word(&eShortA);
	uint16_t longA = eeprom_read_word(&eLongB);
	
	sei();

	PORTB = PORTB^ (1<<PB3);
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
