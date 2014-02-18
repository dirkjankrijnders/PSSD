MMCU=attiny85
CC=avr-gcc
OBJCPY=avr-objcopy
AVRDUDE=avrdude
AVRSIZE=avr-size

compile:
	$(CC) -mmcu=$(MMCU) -Os -g cmd.c ../../src/slave.c ../../src/cmd.c -o cmd.elf
	$(OBJCPY) -j .text -j .data -O ihex cmd.elf cmd.hex
#	$(OBJCPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex cmd.elf cmd.eeprom.hex
	$(AVRSIZE) -A -t cmd.elf

program:
	$(AVRDUDE) -p $(MMCU) -v -e -U flash:w:cmd.hex

clean:
	rm -f *.o *.elf *.hex
