# 	makefile - a part of avr-ds18b20 library
#
#	Copyright (C) 2016 Jacek Wieczorek
#
#	This software may be modified and distributed under the terms
#	of the MIT license.  See the LICENSE file for details.


F_CPU = 16000000UL
MCU = atmega32
MODULES = obj/spi.o obj/uart.o obj/tmp/nrf24l01.o 
MODULE_ROMSEARCH = 

CC = avr-gcc
CFLAGS = -Wall -DUSART2_ENABLED -DF_CPU=$(F_CPU) -mmcu=$(MCU) -Os -std=gnu99

LD = avr-ld
LDFLAGS =

ifneq ($(MAKECMDGOALS),clean)
ifndef F_CPU
$(error F_CPU is not set!)
endif
ifndef MCU
$(error MCU is not set!)
endif
endif



all: obj/nrf24l01.o lib/libnrf24l01.a end request


lib/libnrf24l01p.a: obj/nrf24l01p.o
	avr-ar -cvq lib/libnrf24l01p.a obj/nrf24l01p.o
	avr-ar -t  lib/libnrf24l01p.a

lib/libnrf24l01.a: obj/nrf24l01.o
	avr-ar -cvq lib/libnrf24l01.a obj/nrf24l01.o
	avr-ar -t  lib/libnrf24l01.a

obj/nrf24l01p.o: force $(MODULES)
	$(LD) $(LDFLAGS) -r $(MODULES) -o obj/nrf24l01p.o

obj/nrf24l01.o: force $(MODULES)
	$(LD) $(LDFLAGS) -r $(MODULES) -o obj/nrf24l01.o

obj/spi.o: ./spi/spi.c ./spi/spi.h
	$(CC) $(CFLAGS) -c ./spi/spi.c -o obj/spi.o

obj/uart.o: ./uart/uart.c ./uart/uart.h
	$(CC) $(CFLAGS) -c ./uart/uart.c -o obj/uart.o

obj/tmp/nrf24l01p.o: nrf24l01p/nrf24l01.c nrf24l01p/nrf24l01.h 
	$(CC) $(CFLAGS) -I./nrf24l01p -c nrf24l01p/nrf24l01.c -o obj/tmp/nrf24l01p.o

obj/tmp/nrf24l01.o: nrf24l01/nrf24l01.c nrf24l01/nrf24l01.h
	$(CC) $(CFLAGS) -c nrf24l01/nrf24l01.c -o obj/tmp/nrf24l01.o

force: clean
	-mkdir obj
	-mkdir obj/tmp
	-mkdir lib

clean:
	-rm -rf obj
	-rm -rf lib

end:
	avr-size -C --mcu=$(MCU) obj/nrf24l01.o

#install:
#	cp *.h /home/odroid/usr/include/nrf/
#	cp lib/*.a /home/odroid/usr/lib/

#HEXOBJ=sender
HEXOBJ=blockm
#HEXOBJ=reciver
#HEXOBJ=request
#HEXOBJ=dynload

request:
	avr-gcc -mmcu=$(MCU) -DUSART2_ENABLED -fpack-struct -fshort-enums -ffunction-sections -DF_CPU=$(F_CPU) -Os -o $(HEXOBJ).o $(HEXOBJ).c -I. -I./uart -I./spi -I./nrf24l01 -lnrf24l01 -L./lib
	avr-objcopy -O ihex $(HEXOBJ).o $(HEXOBJ).hex


blockm:
	avr-gcc -mmcu=$(MCU) -std=gnu99 -DF_CPU=$(F_CPU) -Os -o $(HEXOBJ).o $(HEXOBJ).c obj/uart.o -I. -I./uart -I./nrf24l01p 
	avr-objcopy -O ihex $(HEXOBJ).o $(HEXOBJ).hex

install:
	avrdude -p atmega32 -c odroid_c1+ -U flash:w:$(HEXOBJ).hex


