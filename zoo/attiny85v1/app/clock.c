#include <util/delay.h>
#include <avr/io.h>

void delay_us(int delay)
{
	uint16_t i;

	for (i = 0; i < delay; i++){
		_delay_us(1);
	}
}

void delay_ms(int delay)
{
	uint16_t i;

	for (i = 0; i < delay; i++){
		_delay_ms(1);
	}
}
