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

#define SHORT_B_REG_L 0x21
#define SHORT_B_REG_H 0x22
#define LONG_B_REG_L 0x23
#define LONG_B_REG_H 0x24
#define SPEED_B_REG 0x25

#define LAST_A_REG_L 0x31
#define LAST_A_REG_H 0x32
#define LAST_B_REG_L 0x41
#define LAST_B_REG_H 0x42

#define POSITION_A_REG 0xE0
#define POSITION_B_REG 0xF0

#define NULL_REGISTER 0xFF


#endif
