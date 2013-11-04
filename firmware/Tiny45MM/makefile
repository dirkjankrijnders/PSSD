# makefile, written by guido socher
MCU=attiny45
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
REMOVE = rm -f

AVRINC=C:/WinAVR/avr/include/avr
AVRINCINT=C:/WinAVR/avr/include
OUTPUTDIR=Out
OBJLIBDIR=Obj

CFLAGS=-g -mmcu=$(MCU) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
          -std=gnu99 -Wall  -Wstrict-prototypes -Os  -I$(OBJLIBDIR)
ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp -Wall
  
#For usbasp
ISP=usbasp
PORT=/dev/usb/ttyUSB0

#-------------------
.PHONY: all
all: mm_example.hex
#-------------------
mm_example.hex : mm_example.out 
	$(OBJCOPY) -R .eeprom -O ihex $(OUTPUTDIR)/mm_example.out $(OUTPUTDIR)/mm_example.hex 
	avr-size $(OUTPUTDIR)/mm_example.out
	@echo " "
	@echo "Expl.: data=initialized data, bss=uninitialized data, text=code"
	@echo " "

mm_example.out : mm_example.o
	$(CC) $(CFLAGS) -mno-interrupts -o $(OUTPUTDIR)/mm_example.out -Wl,-Map,$(OUTPUTDIR)/mm_example.map \
	$(OUTPUTDIR)/mm_example.o $(OBJLIBDIR)/mm_mod1.o $(OBJLIBDIR)/mm_mod2.o   

#-------------------
mm_example.o : mm_example.c $(OBJLIBDIR)/mm_module.h 
	$(CC) $(CFLAGS) -Os -c -o $(OUTPUTDIR)/mm_example.o mm_example.c 

#-------------------
# avrdude
program: mm_example.hex
	avrdude -p attiny2313 -c stk200 -P lpt1 -e -U flash:w:$(OUTPUTDIR)/mm_example.hex

#-------------------
# Use the ISP USB programmer 
usbasp: clean all mm_example.hex
	avrdude  -p $(MCU) -c ${ISP} -P ${PORT} -U flash:w:$(OUTPUTDIR)/mm_example.hex

#-------------------
# Use the MySmartLight programmer  
mysmart: all
	avrdude  -p $(MCU) -i 70 -P com2 -c stk500v2 -U flash:w:$(OUTPUTDIR)/mm_example.hex
	
#-------------------
clean:
	$(REMOVE) $(OUTPUTDIR)/*.o 
	$(REMOVE) $(OUTPUTDIR)/*.hex
	$(REMOVE) $(OUTPUTDIR)/*.map 
	$(REMOVE) $(OUTPUTDIR)/*.out
	$(REMOVE) $(OUTPUTDIR)/*.lss
#-------------------
