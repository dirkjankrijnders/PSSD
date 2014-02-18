/*

Copyright 2012 Niclas Hoyer    <niclas at verbugt.de>

This file is part of Usitwi.

Usitwi is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Usitwi is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Usitwi.  If not, see <http://www.gnu.org/licenses/>.

*/

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t cmd_buffer[7];
volatile int8_t  cmd_args = 0;
volatile int8_t  cmd_rets = 0;
void           (*cmd_function)();

void ledOn() {
	PORTB ^= (1 << PB1);
}

void get42() {
	cmd_buffer[0] = 0x42;
}

inline void cmd_lookup(uint8_t cmd) {
	if        ( cmd == 0x01 ) {
		cmd_args     = -1;
		cmd_rets     = -1;
		cmd_function = ledOn;
	} else if ( cmd == 0x02 ) {
		cmd_args     = -1;
		cmd_rets     = 0;
		cmd_function = get42;
	} else {
		cmd_function = 0;
	}
}

uint8_t usitwi_address = 0x42;

int main() {
	DDRB = (1 << PB1);
	usitwi_init();
	sei();
	while (1) {
	}
}


