/*

Copyright 2007 Donald R. Blake <donblake at worldnet.att.net>
Copyright 2010 Jochen Toppe    <jochen.toppe at jtoee.com>
Copyright 2011 Ben Galvin      <bgalvin at fastmail.fm>
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

#include "target.h"

#define SET_USI_TO_SEND_ACK() { \
	/* prepare ACK */ \
	USIDR = 0; \
	/* set SDA as output */ \
	DDR_USI |= ( 1 << PORT_USI_SDA ); \
	/* clear all interrupt flags, except Start Cond */ \
	USISR = \
		( 0 << USI_START_COND_INT ) | \
		( 1 << USIOIF ) | ( 1 << USIPF ) | \
		( 1 << USIDC )| \
		/* set USI counter to shift 1 bit */ \
		( 0x0E << USICNT0 ); \
}

#define SET_USI_TO_READ_ACK() { \
	/* set SDA as input */ \
	DDR_USI &= ~( 1 << PORT_USI_SDA ); \
	/* prepare ACK */ \
	USIDR = 0; \
	/* clear all interrupt flags, except Start Cond */ \
	USISR = \
		( 0 << USI_START_COND_INT ) | \
		( 1 << USIOIF ) | \
		( 1 << USIPF ) | \
		( 1 << USIDC ) | \
		/* set USI counter to shift 1 bit */ \
		( 0x0E << USICNT0 ); \
}

#define SET_USI_TO_TWI_START_CONDITION_MODE() { \
	USICR = \
		/* enable Start Condition Interrupt, disable Overflow Interrupt */ \
		( 1 << USISIE ) | ( 0 << USIOIE ) | \
		/* set USI in Two-wire mode, no USI Counter overflow hold */ \
		( 1 << USIWM1 ) | ( 0 << USIWM0 ) | \
		/* Shift Register Clock Source = External, positive edge */ \
		/* 4-Bit Counter Source = external, both edges */ \
		( 1 << USICS1 ) | ( 0 << USICS0 ) | ( 0 << USICLK ) | \
		/* no toggle clock-port pin */ \
		( 0 << USITC ); \
	USISR = \
		/* clear all interrupt flags, except Start Cond */ \
		( 0 << USI_START_COND_INT ) | ( 1 << USIOIF ) | ( 1 << USIPF ) | \
		( 1 << USIDC ) | ( 0x0 << USICNT0 ); \
}

#define SET_USI_TO_SEND_DATA() { \
	/* set SDA as output */ \
	DDR_USI |=  ( 1 << PORT_USI_SDA ); \
	/* clear all interrupt flags, except Start Cond */ \
	USISR =  \
		( 0 << USI_START_COND_INT ) | ( 1 << USIOIF ) | ( 1 << USIPF ) | \
		( 1 << USIDC) | \
		/* set USI to shift out 8 bits */ \
		( 0x0 << USICNT0 ); \
}

#define SET_USI_TO_READ_DATA() { \
	/* set SDA as input */ \
	DDR_USI &= ~( 1 << PORT_USI_SDA ); \
	/* clear all interrupt flags, except Start Cond */ \
	USISR = \
		( 0 << USI_START_COND_INT ) | ( 1 << USIOIF ) | \
		( 1 << USIPF ) | ( 1 << USIDC ) | \
		/* set USI to shift out 8 bits */ \
		( 0x0 << USICNT0 ); \
}

#define STRETCH_CLOCK() { \
	PORT_USI &= ~(1 << PORT_USI_SCL); \
}

#define RELEASE_CLOCK() { \
	PORT_USI |=  (1 << PORT_USI_SCL); \
}
