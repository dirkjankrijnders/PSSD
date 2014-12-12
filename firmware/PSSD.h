//
//  PSSD.h
//  PSSD-i2c
//
//  Created by Dirkjan Krijnders on 02/06/14.
//
//

#ifndef PSSD_i2c_PSSD_h
#define PSSD_i2c_PSSD_h

#define PSSD 0x02
#define CSMD 0x03

#define TYPE_REG 0x01
#define HW_VERSION_REG 0x02
#define FW_VERSION_REG 0x03
#define NAME_REG 0x04

#define ADD_A_REG 0x10
#define ADD_B_REG 0x20

#define SHORT_A_REG_L 0x11
#define SHORT_A_REG_H 0x12
#define LONG_A_REG_L 0x13
#define LONG_A_REG_H 0x14
#define SPEED_A_REG 0x15
#define PORT_A_REG 0x16

#define SHORT_B_REG_L 0x21
#define SHORT_B_REG_H 0x22
#define LONG_B_REG_L 0x23
#define LONG_B_REG_H 0x24
#define SPEED_B_REG 0x25
#define PORT_B_REG 0x26

#define LAST_A_REG_L 0x31
#define LAST_A_REG_H 0x32
#define LAST_B_REG_L 0x41
#define LAST_B_REG_H 0x42

#define POSITION_A_REG 0xE0
#define POSITION_B_REG 0xF0

#define NULL_REGISTER 0xFF


#define IO0 PA0 // J1, pin 13
#define IO1 PA1 // J2, pin 12
#define IO2 PA2 // J3, pin 11

#define IO3 PB0 // J7, pin 2
#define IO4 PB1 // J8, pin 3

#define PORTIO012 PORTA
#define PINIO012 PINA
#define DDRIO012 DDRA

#define PORTIO34 PORTB
#define PINIO34 PINB
#define DDRIO34 DDRB

#define DEBOUNCE0 GPIOR0
#define DEBOUNCE1 GPIOR1

#define MANUALSTATE GPIOR2
#endif
