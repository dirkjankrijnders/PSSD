file=mm_servo_double_0_03.hex
lfuse=0xff
hfuse=0x99

AVRDUDE=avrdude
AVRPART=t2313

verify:
	$(AVRDUDE) -p $(AVRPART) -U flash:v:$(file)

flash: $(file)
	$(AVRDUDE) -p $(AVRPART) -U flash:w:$(file)

fuses:
	$(AVRDUDE) -p $(AVRPART) -U lfuse:w:$(lfuse):m
	$(AVRDUDE) -p $(AVRPART) -U hfuse:w:$(hfuse):m

upload: flash fuses

