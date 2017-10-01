#include <util/delay.h>
#include <avr/io.h>

void delay_us(int delay)
{
	/* FIXME: very coarse delay_us implementation
	 * because each MCU instruction takes ~1us
	 *
	 * - not suitable for w1 operations
	 * - suitable for nRF24 ops
	 */
	for (int i = 0; i < delay; i++)
		_delay_us(1);
}

void delay_ms(int delay)
{
	for (int i = 0; i < delay; i++)
		_delay_ms(1);
}
