/*#define N 64
 #define TOP 1/(N*50/F_CPU)-1
 #define TOP 4999
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "usitwir/src/slave.h"
//#include "mm_module.h"
#include "mm1acc/mm1acc.h"

#include "PSSD.h"

#if defined( __AVR_ATtiny2313__ )
#define t2313
#define OCA OCR1A
#define PORT_FET PORTD
#define P_FET PD0
#endif

#if defined( __AVR_ATtiny25__ ) | \
defined( __AVR_ATtiny45__ ) | \
defined( __AVR_ATtiny85__ )
#define t25
#endif

#if defined( __AVR_ATtiny24__ ) | \
defined( __AVR_ATtiny44__ ) | \
defined( __AVR_ATtiny84__ )
#define t24
#define PORT_FET PORTA
#define P_FET PA7
#endif

#ifndef BOARD
#define BOARD PSSD
#endif

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


uint8_t usitwi_address = 0x10;
uint8_t currentRegister = NULL_REGISTER;
uint8_t temp = 0;

uint16_t EEMEM eShortA = 1250;
uint16_t EEMEM eShortB = 1250;
uint16_t EEMEM eLongA = 1500;
uint16_t EEMEM eLongB = 1500;
uint8_t  EEMEM eAddA = ADD_A ;
uint8_t  EEMEM eAddB = ADD_B ;
uint8_t  EEMEM ePortA = PORT_A ;
uint8_t  EEMEM ePortB = PORT_B ;
uint8_t	 EEMEM eSpeedA = 2;
uint8_t	 EEMEM eSpeedB = 2;

uint16_t EEMEM eLastA = 1250;
uint16_t EEMEM eLastB = 1250;

uint8_t volatile AddA    = 0;
uint8_t volatile PortA   = 0;
uint16_t volatile shortA = 0;
uint16_t volatile longA  = 0;

uint8_t volatile AddB    = 0;
uint8_t volatile PortB   = 0;
uint16_t volatile shortB = 0;
uint16_t volatile longB  = 0;
uint8_t volatile speedA  = 1;
uint8_t	volatile speedB	 = 1;

uint8_t volatile loop = 1;
uint16_t targetA;
uint16_t targetB;

uint16_t currentA = 1000;
uint16_t currentB = 1000;

void setup_servo_pwm();

#ifdef t24
ISR(PCINT0_vect) {
	if (PINA & (1 << PA3)) { // Rising flank, programmer attached!
		
		TCCR1B = 0; // Stop servo PWM
		PORTA &= ~(1 << P_FET);
		usitwi_init(); // Enable the I2C interface
	} else { // Falling flank, programmer detached!
		usitwi_deinit();
		loop = 0;
		setup_servo_pwm();
	}
}
#endif

void setup_servo_pwm() {
#ifdef t2313
	TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        // Inverted PWM
	TCCR1B=(1<<CS11)|(1<<WGM12)|(1<<WGM13); // PRESCALER=/8 MODE 14(FAST PWM, TOP=ICR1)
	ICR1 = 25000;
	
	// Measured resolution: 22 uS
	//Make sure OC1n are output
	DDRB|=(1<<PB4)|(1<<PB3); //PWM Pins as Out(1<<PD3)|
	
	DDRD|=(1<<PD0)|(1<<PD6); // GND Fet connection & LED
	
	PORTD |= (1<<PD0) | (1<<PD6);
	
	usitwi_init();
#endif
#ifdef t25 // Won't work because the resolution is too low
	TCCR1 |= (1<<CS12)|(1<<CS11)|(1<<CS10); // Prescaler to /1024 => f_TCK1 =~ 8kHz (7812.5 Hz)
	TCCR1 |= (1<<PWM1A)|(1<<COM1A1)|(0<<COM1A0); // Set on $00, clear on match
	GTCCR |= (1<<PWM1B)|(1<<COM1B1)|(0<<COM1B0); // Set on $00, clear on match
	
	DDRB |= (1 << PB1) | (1 << PB4);
    usitwi_init();
	
#endif
#ifdef t24 //
	TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        // Inverted PWM
	TCCR1B=(1<<CS11)|(1<<WGM12)|(1<<WGM13); // PRESCALER=/8 MODE 14(FAST PWM, TOP=ICR1)
	ICR1 = 20000; //20000 / 8000000 / 8 = 20 ms
	
	
	DDRA |= (1 << PA5) | (1 << PA6); // Enable the servo pwm channels as output, should be PA5 en PA6
	DDRA |= (1 << P_FET); // Enable GND Fet as output
	
	//	PORT_FET |= (1 << P_FET);
	
	// I2C Programmer attachment:
	PCMSK0 |= (1 << PCINT3);
	GIMSK |= (1 << PCIE0);
#endif
}

void io_init() {
	// SETUP IO 0 & 1 as buttons
	DDRIO012 &= ~((1 << IO0) | (1 << IO0)); //Inputs
	
	// Enable pull-ups, the switch would pull them to ground
	PORTIO012 |= (1 << IO0) | (1 << IO0);
	
	// Set up the DEBOUNCE registers:
	DEBOUNCE0 = 0;
	DEBOUNCE1 = 0;
	
	// Setup IO 3 & 4 as feedback outputs
	DDRIO34 |= ((1 << IO3) | (1 << IO4));
}

int main()  __attribute__ ((noreturn));

int main() {
	acc_data data;
	
	uint8_t FET_state = 1;
	
    /* Initialize the MM module */
	mm1acc_init();
	
	setup_servo_pwm();
	
	io_init();
	
	PORT_FET &= ~(1 << P_FET);
	DDRA |= (1 << PA2); // Pad J1 as output
	sei();
	for (;;) {
		loop = 1;
		AddA = eeprom_read_byte(&eAddA);
		PortA = eeprom_read_byte(&ePortA);
		shortA = eeprom_read_word(&eShortA);
		longA = eeprom_read_word(&eLongA);
		
		AddB = eeprom_read_byte(&eAddB);
		PortB = eeprom_read_byte(&ePortB);
		shortB = eeprom_read_word(&eShortB);
		longB = eeprom_read_word(&eLongB);
		
		speedA = eeprom_read_byte(&eSpeedA);
		speedB = eeprom_read_byte(&eSpeedB);
		targetA = eeprom_read_word(&eLastA);
		targetB = eeprom_read_word(&eLastB);
		
		FET_state = 1;
		
		for(;loop == 1;) {
			if (mm1acc_check(&data)) {
				// New data
				if (data.address == AddA) {
					if (data.function == 1) {
						if (data.port == PortA) {
							PORTA |= (1 << PA2);
							targetA = shortA;
							FET_state = 1;
							eeprom_write_word(&eLastA, shortA);
						} else if (data.port == PortA + 1) {
							PORTA |= (1 << PA2);
							targetA = longA;
							FET_state = 1;
							eeprom_write_word(&eLastA, longA);
						}
					}
				}
				if (data.address == AddB) {
					if (data.function == 1) {
						if (data.port == PortB) {
							targetB = shortB;
							FET_state = 1;
							eeprom_write_word(&eLastB, shortB);
						} else if (data.port == PortB + 1) {
							targetB = longB;
							FET_state = 1;
							eeprom_write_word(&eLastB, longB);
						}
					}
					
				}
			}
#ifdef t24
			if (currentA < targetA) {
				currentA = currentA + speedA;
				if (currentA >= targetA) {
					currentA = targetA;
				}
				OCR1A = currentA;
			} else if (currentA > targetA) {
				currentA = currentA - speedA;
				if (currentA <= targetA) {
					currentA = targetA;
				}
				OCR1A = currentA;
			}
			if (currentB < targetB) {
				currentB = currentB + speedB;
				if (currentB >= targetB) {
					currentB = targetB;
				}
				OCR1B = currentB;
			} else if (currentB > targetB) {
				currentB = currentB - speedB;
				if (currentB <= targetB) {
					currentB = targetB;
				}
				OCR1B = currentB;
			}
#else
			OCR1A = targetA;
			OCR1B = targetB;
#endif
			
			if (FET_state > 0) {
				PORT_FET |= (1 << P_FET);
			}
			
			if (FET_state > 9) {
				FET_state++;
			}
			
			if (FET_state > 50) {
				FET_state = 0;
			}
			
			if (FET_state == 0) {
				PORT_FET &= ~(1 << P_FET);
			}
			
			if (currentA == targetA && currentB == targetB && FET_state > 0 && FET_state < 10) FET_state = 10;
			_delay_ms(1);
			PORTA &= ~(1 << PA2);

			DEBOUNCE0 = DEBOUNCE0 << 1 | (PINIO012 & ((1 << IO0) ? 1 :0));
			DEBOUNCE1 = DEBOUNCE1 << 1 | (PINIO012 & ((1 << IO1) ? 1 :0));
			
			if (DEBOUNCE0 == 255 && ~(MANUALSTATE & 1)) {
				// Button 0 pressed!
				MANUALSTATE &= 1;
				if (targetA == shortA){
					PORTA |= (1 << PA2);
					targetA = longA;
					FET_state = 1;
					eeprom_write_word(&eLastA, longA);
				} else {
					PORTA |= (1 << PA2);
					targetA = shortA;
					FET_state = 1;
					eeprom_write_word(&eLastA, shortA);
				}
			}
			if (DEBOUNCE0 == 0)
				MANUALSTATE &= 0b11111110;
			
			if (DEBOUNCE1 == 255 && ~(MANUALSTATE & 2)) {
				// Button 1 pressed!
				MANUALSTATE &= 2;
			}
			if (DEBOUNCE1 == 0)
				MANUALSTATE &= 0b11111101;
			
			if (currentA == shortA)
				PORTIO34 |= (1 << IO3);
			if (currentA == longA)
				PORTIO34 &= ~(1 << IO3);

			if (currentB  == shortB)
				PORTIO34 |= (1 << IO4);
			if (currentB == longB)
				PORTIO34 &= ~(1 << IO4);
		} // While
	}
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
			return BOARD; // First I2C version
		case NAME_REG:
			return 0xFF;
		case ADD_A_REG:
			return eeprom_read_byte(&eAddA);
		case ADD_B_REG:
			return AddB;
		case PORT_A_REG:
			return eeprom_read_byte(&ePortA);
		case PORT_B_REG:
			return eeprom_read_byte(&ePortB);
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
		case LAST_A_REG_L:
			return (eeprom_read_word(&eLastA)) & 0xFF;
		case LAST_A_REG_H:
			return (eeprom_read_word(&eLastA) >> 8) & 0xFF;
		case LAST_B_REG_L:
			return (eeprom_read_word(&eLastB)) & 0xFF;
		case LAST_B_REG_H:
			return (eeprom_read_word(&eLastB) >> 8) & 0xFF;
#ifdef t24
		case SPEED_A_REG:
			return speedA;
		case SPEED_B_REG:
			return speedB;
#endif
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
				eeprom_write_byte(&eAddA, value);
				break;
			case ADD_B_REG:
				eeprom_write_byte(&eAddB, value);
				break;
			case PORT_A_REG:
				eeprom_write_byte(&ePortA, value);
				break;
			case PORT_B_REG:
				eeprom_write_byte(&ePortB, value);
				break;
			case SHORT_A_REG_L:
				temp = value;
				return;
			case SHORT_A_REG_H:
				shortA = temp + (value << 8);
				eeprom_write_word(&eShortA, shortA);
				eeprom_write_word(&eLastA, shortA);
				break;
			case SHORT_B_REG_L:
				temp = value;
				return;
			case SHORT_B_REG_H:
				shortB = temp + (value << 8);
				eeprom_write_word(&eShortB, shortB);
				eeprom_write_word(&eLastB, shortB);
				break;
			case LONG_A_REG_L:
				temp = value;
				return;
			case LONG_A_REG_H:
				longA = temp + (value << 8);
				eeprom_write_word(&eLongA, longA);
				eeprom_write_word(&eLastA, longA);
				break;
			case LONG_B_REG_L:
				temp = value;
				break;
			case LONG_B_REG_H:
				longB = temp + (value << 8);
				eeprom_write_word(&eLongB, longB);
				eeprom_write_word(&eLastB, longB);
				break;
			case POSITION_A_REG:
				if (value == 0x00) {
					eeprom_write_word(&eLastA, shortA);
				} else if (value == 0x01) {
					eeprom_write_word(&eLastA, longA);
				}
				break;
			case POSITION_B_REG:
				if (value == 0x00) {
					eeprom_write_word(&eLastB, shortB);
				} else if (value == 0x01) {
					eeprom_write_word(&eLastB, longB);
				}
				break;
#ifdef t24
			case SPEED_A_REG:
				eeprom_write_byte(&eSpeedA, value);
				break;
			case SPEED_B_REG:
				eeprom_write_byte(&eSpeedB, value);
				break;
#endif
			default:
				break;
		}
		loop = 0;
		currentRegister++;
	}
}
