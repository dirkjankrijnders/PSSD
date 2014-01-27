//
//  mm1sw.c
//  PSSD-i2c
//
//  Created by Dirkjan Krijnders on 01/12/13.
//
//

//#include <avr/timer.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define MMCOUNTER TCNT0
#define MMPIN_INT INT0_vect
#define BIT_COMP TIMER0_COMPA_vect
#define PAUSE_OVF TIMER0_OVF_vect
#define MMPORT PIND
#define MMPIN PD2
#define LED_PIN PB1

uint32_t volatile byte;
uint8_t volatile ntrits;
uint8_t volatile dataready;

ISR(MMPIN_INT){
	MMCOUNTER = 0;
	// Set prescaler to /8
	TCCR0B |= (1 << CS01);
//	PORTB = PORTB ^ (1 << LED_PIN); // Toggle the LED
};


ISR(TIMER0_COMPA_vect)
{
	byte = byte << 1;
	byte |= MMPORT & MMPIN;
	MMCOUNTER = 0;
	TCCR0B = 0; //stop the timer
	ntrits++;
	if (ntrits == 18)
	{
		dataready = 1;
		ntrits = 0;
	};
};

ISR(TIMER0_OVF_vect)
{
	ntrits = 0;
	byte = 0;
//	dataready = 0;
	TCCR0B = 0; //stop the timer
	PORTB = PORTB ^ (1 << LED_PIN);  // Toggle the LED
}

/* Poll the specified string out the debug port.
void debug_puts(const char *str) {
	const char *c;
	
	for(c = str; *c; c++)
		special_output_port = *c;
}*/

void USARTInit(unsigned int ubrr_value) { // is UBRR>255 supported?
    //Set Baud rate
    UBRRH = (unsigned char)(ubrr_value >> 8);
    UBRRL = (unsigned char)(ubrr_value & 255);
    // Frame Format: asynchronous, no parity, 1 stop bit, char size 8
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
    //Enable The receiver and transmitter
    UCSRB = (1 << RXEN) | (1 << TXEN);
}

void USARTWriteChar(char data) { // blocking
    while(!(UCSRA & (1<<UDRE))) {};
	UDR=data;
}


/* The Motorola protocol for switches is fairly simple if you don't consider the
 "trinary" side of things. A bit is represented by:
 1: ‾_______
 0: ‾‾‾‾‾‾‾_
    |--->
 So if we check the level at >, e.g. 4 * 13 = 52us after a positive rising edge 
 we can establish the bit value.
 
 Pauses happen between packets and are longer (shortest = 1025 us). So after the
 a bit we reset the counter and start counting again. Now if it overflows, we 
 restart the "byte"
 
 Assuming an 8 bit counter or steps should be 4 us. At 10MHz and a clock divider
 of 4 would be excellent. The best we have is 8, giving us 52 us = 26 steps and 
 a pause measurement of 512 us which will work as well. If not we could "misuse"
 the TOV0F as ninth bit.
 
 For the attiny2313:
 Clock select:
 CS02:0 = 010
 TCCR0B |= (1 << CS01);
 
 Overflow interupt:
 
 OCRA:
 OCF0A = 26;
// TCCR0B |= (1 << FOC0A);
 TIMSK |= (1 << TOIE0) | (1 << OCIE0A)
 
 Operation mode (normal):
 WGM02:0 = 000
 
 */
 
 
 
void setup_timer0() {
	// Output Compare Register A
	OCR0A = 26;
	// Timer/Counter0 Overflow Interrupt Enable and Timer/Counter0 Output Compare
	// Match A Interrupt Enable
	TIFR |= (1<<TOV0);
	TIMSK |= (1 << TOIE0) | (1 << OCIE0A);
	// Set prescaler to /8
//	TCCR0B |= (1 << CS01);
}

int main() {
	DDRB |= (1<<LED_PIN); // Set as output
	PORTB |= (1<<LED_PIN); // Set as source
	//PINB |= (1<<LED_PIN);
	USARTInit(15); // 14.7456 MHz / (16 * 57600 baud) - 1 = 15.00x
	USARTWriteChar('h');
	GIMSK |= (1 << INT0);
	MCUCR |= (1 << ISC01)|(1 << ISC00);
	
	setup_timer0();
	sei();
	while(1){
	//	debug_puts((char*)&TCNT0);
		if (dataready) {
			USARTWriteChar(byte & 0xFF);
			byte = byte >> 8;
			USARTWriteChar(byte & 0xFF);
			byte = byte >> 8;
			USARTWriteChar(byte & 0xFF);
			dataready = 0;
		}
//		USARTWriteChar('e');
	}
	return 1;
}