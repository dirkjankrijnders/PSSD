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
#include "../../src/slave.h"

#define LED PB1

#define NULL_REGISTER 0xFF

static volatile uint8_t register1 = 0;
static volatile uint8_t register2 = 0;

uint8_t currentRegister = NULL_REGISTER;

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
		case 0:
			return register1;
		break;
		case 1:
			return register2;
		break;
		default:
			return 0xFF;
		break;
	}
}

void usitwi_onWrite(uint8_t value) {
	if (currentRegister == NULL_REGISTER) {
		currentRegister = value;
	} else {
		switch(currentRegister) {
			case 0:
				register1 = value;
			break;
			case 1:
				register2 = value;
			break;
		}
	}
}

uint8_t usitwi_address = 0x42;

int main() {
	DDRB |= (1 << LED);

	usitwi_init();

	sei();

	while (1) {
		if (register1 == register2)
			PORTB |= 1 << LED;
		else
			PORTB &= ~(1 << LED);
	}
}
