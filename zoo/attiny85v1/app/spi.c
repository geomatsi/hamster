/*
 * Hardware SPI master library for ATtiny44/84 and ATtiny45/85
 *
 * Jack Christensen 24Oct2013
 *
 * CC BY-SA
 * This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0 or send a letter
 * to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 */

#include <util/atomic.h>

#include "spi.h"

/* */

void spi_init(void)
{

	/* set the USCK pin as output */
	DDRB |= _BV(DDB2);

	/* set the DO pin as output */
	DDRB |= _BV(DDB1);

	/* set the DI pin as input */
	DDRB &= ~_BV(DDB0);

	/* setup USI */
	USICR &= ~(_BV(USISIE) | _BV(USIOIE) | _BV(USIWM1));
	USICR |= _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);
}

void spi_set_mode(uint8_t mode)
{
	switch (mode) {
		case SPI_MODE_0:
			USICR &= ~_BV(USICS0);
			break;
		case SPI_MODE_1:
			USICR |= _BV(USICS0);
			break;
		case SPI_MODE_2:
		case SPI_MODE_3:
			/* not supported: fall back to SPI_MODE_0 */
			USICR &= ~_BV(USICS0);
			break;
		default:
			/* unknown: fall back to SPI_MODE_0 */
			USICR &= ~_BV(USICS0);
			break;
	}
}

void spi_deinit(void)
{
	/* turn off USI */
	USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}

uint8_t spi_fast_shift(uint8_t data)
{
	USIDR = data;

	/* clear counter and counter overflow interrupt flag */
	USISR = _BV(USIOIF);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		/* ensure a consistent clock period */
		while ( !(USISR & _BV(USIOIF)) )
			USICR |= _BV(USITC);
	}

	return USIDR;
}
