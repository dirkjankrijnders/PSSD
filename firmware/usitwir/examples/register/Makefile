MMCU=attiny85
CC=avr-gcc
OBJCPY=avr-objcopy
AVRDUDE=avrdude
AVRSIZE=avr-size

compile:
	$(CC) -mmcu=$(MMCU) -Os -g register.c ../../src/slave.c -o register.elf
	$(OBJCPY) -j .text -j .data -O ihex register.elf register.hex
#	$(OBJCPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex register.elf register.eeprom.hex
	$(AVRSIZE) -A -t register.elf

example-cmd:
	$(CC) -mmcu=$(MMCU) -Os -g examples/cmd.c src/slave.c src/cmd.c -o out/cmd.elf
	$(OBJCPY) -j .text -j .data -O ihex out/cmd.elf out/cmd.hex
	$(OBJCPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex out/cmd.elf out/cmd.eeprom.hex
	$(AVRSIZE) -A -t out/cmd.elf

program:
	$(AVRDUDE) -p $(MMCU) -v -e -U flash:w:register.hex

clean:
	rm -f *.o *.elf *.hex
