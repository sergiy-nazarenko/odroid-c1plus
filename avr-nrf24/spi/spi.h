/*
spi lib 0x01

copyright (c) Davide Gironi, 2012

References:
  -  This library is based upon SPI avr lib by Stefan Engelke
     http://www.tinkerer.eu/AVRLib/SPI

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>

//spi ports
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_MISO PB6
#define SPI_MOSI PB5
#define SPI_SCK PB7
#define SPI_SS PB4

extern void spi_init();
extern uint8_t spi_writereadbyte(uint8_t data);

#endif
