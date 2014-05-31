//
//  lcd.h
//  PSSDProgrammer
//
//  Created by Dirkjan Krijnders on 14/03/14.
//
//

#ifndef PSSDProgrammer_lcd_h
#define PSSDProgrammer_lcd_h

void lcd_init();
void lcd_write(unsigned char c);

void lcd_clear(void);
void lcd_puts(const char * s);
void lcd_putch(unsigned char c);
void lcd_goto(unsigned char pos,unsigned char line);

#endif
