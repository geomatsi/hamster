#include <util/delay.h>
#include <avr/io.h>

void delay_us(int delay)
{
	uint16_t i;

	for (i = 0; i < delay; i++){
		/* wait (4 x 4) cycles = wait 16 cycles ~= 1us */
		_delay_loop_2(4);
	}
}

void delay_ms(int delay)
{
	uint16_t i;

	for (i = 0; i < delay; i++){
		/* wait (4000 x 4) cycles = wait 16000 cycles ~= 1ms */
		_delay_loop_2(4000);
	}
}
