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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "slave.h"
#include "slave/target.h"
#include "slave/macros.h"

/*
 * # States
 */
typedef enum {
	USI_SLAVE_CHECK_ADDRESS,
	USI_SLAVE_SEND_DATA,
	USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA,
	USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA,
	USI_SLAVE_REQUEST_DATA,
	USI_SLAVE_GET_DATA_AND_SEND_ACK
} overflowState_t;

static volatile overflowState_t overflowState;

/*
 * # TWI Interface
 */
extern uint8_t usitwi_address;
extern uint8_t usitwi_onRead();
extern void    usitwi_onWrite(uint8_t value);
extern void    usitwi_onStart(uint8_t rw);
extern void    usitwi_onStop();

/*
 * # Initializing
 * initialise USI for TWI slave mode
 */
void usitwi_init() {

	// ## Mode of Operation
	// In Two Wire mode (USIWM1, USIWM0 = 1X), the slave USI will pull SCL
	// low when a start condition is detected or a counter overflow (only
	// for USIWM1, USIWM0 = 11).  This inserts a wait state.  SCL is released
	// by the ISRs (USI_START_vect and USI_OVERFLOW_vect).

	// Set SCL and SDA as output
	DDR_USI |= ( 1 << PORT_USI_SCL ) | ( 1 << PORT_USI_SDA );

	// set SCL high
	PORT_USI |= ( 1 << PORT_USI_SCL );

	// set SDA high
	PORT_USI |= ( 1 << PORT_USI_SDA );

	// Set SDA as input
	DDR_USI &= ~( 1 << PORT_USI_SDA );

	USICR =
		// enable Start Condition Interrupt
		( 1 << USISIE ) |
		// disable Overflow Interrupt
		( 0 << USIOIE ) |
		// set USI in Two-wire mode, no USI Counter overflow hold
		( 1 << USIWM1 ) | ( 0 << USIWM0 ) |
		// Shift Register Clock Source = external, positive edge
		// 4-Bit Counter Source = external, both edges
		( 1 << USICS1 ) | ( 0 << USICS0 ) | ( 0 << USICLK ) |
		// no toggle clock-port pin
		( 0 << USITC );

	// clear all interrupt flags and reset overflow counter
	USISR = ( 1 << USI_START_COND_INT ) | ( 1 << USIOIF ) | ( 1 << USIPF ) | ( 1 << USIDC );

}

/*
 * # USI Start Condition
 */
ISR(USI_START_VECTOR) {
	// set default starting conditions for new TWI package
	overflowState = USI_SLAVE_CHECK_ADDRESS;

	// set SDA as input
	DDR_USI &= ~( 1 << PORT_USI_SDA );

	// wait for SCL to go low to ensure the Start Condition has completed (the
	// start detector will hold SCL low ) - if a Stop Condition arises then leave
	// the interrupt to prevent waiting forever - don't use USISR to test for Stop
	// Condition as in Application Note AVR312 because the Stop Condition Flag is
	// going to be set from the last TWI sequence
	while (
		// SCL his high
		( PIN_USI & ( 1 << PIN_USI_SCL )) &&
		// and SDA is low
		!(( PIN_USI & ( 1 << PIN_USI_SDA )))
	);

	if ( !( PIN_USI & (1 << PIN_USI_SDA )) ) {

		// a Stop Condition did not occur
		USICR =
			// keep Start Condition Interrupt enabled to detect RESTART
			( 1 << USISIE ) |
			// enable Overflow Interrupt
			( 1 << USIOIE ) |
			// set USI in Two-wire mode, hold SCL low on USI Counter overflow
			( 1 << USIWM1 ) | ( 1 << USIWM0 ) |
			// Shift Register Clock Source = External, positive edge
			// 4-Bit Counter Source = external, both edges
			( 1 << USICS1 ) | ( 0 << USICS0 ) | ( 0 << USICLK ) |
			// no toggle clock-port pin
			( 0 << USITC );

	} else {

		// a Stop Condition did occur
		usitwi_onStop();

		USICR =
			// enable Start Condition Interrupt
			( 1 << USISIE ) |
			// disable Overflow Interrupt
			( 0 << USIOIE ) |
			// set USI in Two-wire mode, no USI Counter overflow hold
			( 1 << USIWM1 ) | ( 0 << USIWM0 ) |
			// Shift Register Clock Source = external, positive edge
			// 4-Bit Counter Source = external, both edges
			( 1 << USICS1 ) | ( 0 << USICS0 ) | ( 0 << USICLK ) |
			// no toggle clock-port pin
			( 0 << USITC );

	}

	USISR =
		// clear interrupt flags - resetting the Start Condition Flag will
		// release SCL
		( 1 << USI_START_COND_INT ) | ( 1 << USIOIF ) |
		( 1 << USIPF ) |( 1 << USIDC ) |
		// set USI to sample 8 bits (count 16 external SCL pin toggles)
		( 0x0 << USICNT0);

}

/*
 * # USI Overflow
 * Handles all the communication.
 * Only disabled when waiting for a new Start Condition.
 */
ISR( USI_OVERFLOW_VECTOR ) {

	uint8_t tmp;

	switch ( overflowState ) {

	// ## Address Mode
	// check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else
	// reset USI
	case USI_SLAVE_CHECK_ADDRESS:
		if ( ( USIDR == 0 ) || ( ( USIDR >> 1 ) == usitwi_address) ) {
			usitwi_onStart(USIDR & 0x01);
			if ( USIDR & 0x01 ) {
				overflowState = USI_SLAVE_SEND_DATA;
			} else {
				overflowState = USI_SLAVE_REQUEST_DATA;
			}
			SET_USI_TO_SEND_ACK( );
		} else {
			SET_USI_TO_TWI_START_CONDITION_MODE( );
		}
	break;

	// ## Master Write Data Mode
	// check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI
	case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
		if ( USIDR ) {
			// if NACK, the master does not want more data
			SET_USI_TO_TWI_START_CONDITION_MODE( );
			return;
		}
		// from here we just drop straight into USI_SLAVE_SEND_DATA if the
		// master sent an ACK

	// ## Slave Send Data Mode
	// copy data from buffer to USIDR and set USI to shift byte
	// next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
	case USI_SLAVE_SEND_DATA:
		STRETCH_CLOCK();
		USIDR = usitwi_onRead();
		overflowState = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
		SET_USI_TO_SEND_DATA( );
		RELEASE_CLOCK();
	break;

	// set USI to sample reply from master
	// next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
	case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
		overflowState = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
		SET_USI_TO_READ_ACK( );
	break;

	// ## Master Read Data Mode:
	// set USI to sample data from master, next USI_SLAVE_GET_DATA_AND_SEND_ACK
	case USI_SLAVE_REQUEST_DATA:
		overflowState = USI_SLAVE_GET_DATA_AND_SEND_ACK;
		SET_USI_TO_READ_DATA( );
	break;

	// copy data from USIDR and send ACK
	// next USI_SLAVE_REQUEST_DATA
	case USI_SLAVE_GET_DATA_AND_SEND_ACK:
		// next USI_SLAVE_REQUEST_DATA
		tmp = USIDR;
		overflowState = USI_SLAVE_REQUEST_DATA;
		SET_USI_TO_SEND_ACK( );
		usitwi_onWrite(tmp);
	break;

	}

}
