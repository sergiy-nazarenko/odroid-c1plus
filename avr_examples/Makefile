


all:
	avr-gcc -mmcu=atmega328p -Os -o blink.o blink.c -std=gnu99
	avr-objcopy -O ihex blink.o blink.hex
install:
	avrdude -p atmega328p -c odroid_c1+ -U flash:w:blink.hex


