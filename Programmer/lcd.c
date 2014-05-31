//
//  lcd.c
//  PSSDProgrammer
//
//  Created by Dirkjan Krijnders on 14/03/14.
//
#include <util/delay.h>
#include <avr/io.h>

//  4-bit mode
#define PIN_EN PINC
#define P_EN PC2
#define PORT_RS PORTC
#define P_RS PC3

#define PORT_D4 PORTC
#define P_D4 PC1
#define PORT_D567 PORTB
#define P_D5 PB5
#define P_D6 PB4
#define P_D7 PB3

#define LCD_STROBE PORTC |= (1 << P_EN); _delay_ms(10);PORTC &= ~(1 << P_EN);

void
lcd_write(unsigned char c)
{
    if(c & 0x80) PORT_D567 |= (1 << P_D7); else PORT_D567 &= ~(1 << P_D7);
    if(c & 0x40) PORT_D567 |= (1 << P_D6); else PORT_D567 &= ~(1 << P_D6);
    if(c & 0x20) PORT_D567 |= (1 << P_D5); else PORT_D567 &= ~(1 << P_D5);
    if(c & 0x10) PORT_D4 |= (1 << P_D4); else PORT_D4 &= ~(1 << P_D4);
    LCD_STROBE;
    if(c & 0x08) PORT_D567 |= (1 << P_D7); else PORT_D567 &= ~(1 << P_D7);
    if(c & 0x04) PORT_D567 |= (1 << P_D6); else PORT_D567 &= ~(1 << P_D6);
    if(c & 0x02) PORT_D567 |= (1 << P_D5); else PORT_D567 &= ~(1 << P_D5);
    if(c & 0x01) PORT_D4 |= (1 << P_D4); else PORT_D4 &= ~(1 << P_D4);
    LCD_STROBE;
    _delay_us(40);
}

void lcd_init(){
	// Set LCD Pins as output
	DDRC |= (1 << PC1) | (1 << PC2) | (1 << PC3);
	DDRB |= (1 << PB3) | (1 << PB4) | (1 << PB5);
	
	// Pull RS Line low
	PORT_RS &= ~(1 << P_RS);
	
	_delay_ms(15); // power on delay
	
	// Init display
	PORT_D4 |= (1 << P_D4);
	PORT_D567 |= (1 << P_D5);
	
	LCD_STROBE;
	
	_delay_ms(5);
	
	LCD_STROBE;
	
	_delay_ms(100);
	
	LCD_STROBE;
	
	_delay_ms(5);
	PORT_D4 &= ~(1 << P_D4); // Set 4 bit mode
	
	LCD_STROBE;
	_delay_ms(40);
	
	lcd_write(0x28);// 4 bit mode, 1/16 duty, 5x8 font, 2lines
    lcd_write(0x0C);// display on
    lcd_write(0x06);// entry mode advance cursor
    lcd_write(0x01);// clear display and reset cursor
}

void
lcd_clear(void)
{
    PORT_RS &= ~(1 << P_RS);
    lcd_write(0x1);
    _delay_ms(2);
}
/* write a string of chars to the LCD */
void
lcd_puts(const char * s)
{
    PORT_RS |= (1 << P_RS);    // write characters
    while(*s) lcd_write(*s++);
}
/* write one character to the LCD */
void
lcd_putch(unsigned char c)
{
    PORT_RS |= (1 << P_RS);    // write characters
    lcd_write(c);
}
/*
 * Go to the specified position
 */
void
lcd_goto(unsigned char pos,unsigned char line)
{
    PORT_RS &= ~(1 << P_RS);
    if (line==0)
		lcd_write(0x80 + pos);
    else
		lcd_write(0x80 + pos+ 0x40);
}

