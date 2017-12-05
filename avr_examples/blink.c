#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

int main(void)
{
	DDRB |= 1<<0;
	PORTB |= 1<<0;
	
	while(1)
	{
		PORTB &= ~(1<<0);
		_delay_ms(1100);
		PORTB |= (1<<0);
		
		_delay_ms(1000);
	}
	return 0;
}
