#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#define BAUD 9600


#include <util/setbaud.h>

/* cpp
FILE * uart_str; 
static int uart_putchar(char c , FILE *stream);
static int uart_getchar(FILE *stream);
*/
#define MYUBRR F_CPU/16/BAUD-1

 //c 328p
void init_uart(void) {
    UBRRH = (uint8_t)(MYUBRR>>8);
    UBRRL = (uint8_t)MYUBRR;

    #if USE_2X
    UCSRA |= _BV(U2X);
    #else
    UCSRA &= ~(_BV(U2X));
    #endif

    UCSRC = (1<<URSEL) | (3<<UCSZ0); // 8-bit data 
    UCSRB = (1<<RXEN) | (1<<TXEN); // Enable RX and TX 
}

void uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
      uart_putchar('\r', stream);
    } 
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}


char uart_getchar(FILE *stream) {
    loop_until_bit_is_set(UCSRA, RXC); // Wait until data exists. 
    return UDR;
}

/*
void init_uart(void)
{
    UBRRH = UBRRH_VALUE;//(unsigned char)(ubrr>>8);
    UBRRL = UBRRL_VALUE;//(unsigned char)ubrr;
    UCSRB = (1<<RXEN)|(1<<TXEN);
    UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}
*/

static int 
uart_putchar32(char c, FILE *stream)
{
    if(c == '\n')
        uart_putchar('\r', stream);
    while( !(UCSRA & (1<<UDRE)) )
        ;
    UDR = c;
    return 0;
}



static int
uart_getchar32(FILE *stream)
{
    while( !(UCSRA & (1<<RXC)) )
        ;
    char data = UDR;
    if(data == '\r')
        data = '\n';
    uart_putchar(data, stream);
    return  data;
}


//FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
//FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
//FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
