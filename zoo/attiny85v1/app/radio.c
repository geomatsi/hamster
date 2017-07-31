#include "delay.h"
#include "radio.h"
#include "spi.h"

/*
 * Pin control functions for libnrf24 library:
 *   - MISO <-> PB0
 *   - MOSI <-> PB1
 *   - SCK  <-> PB2
 *   - CSN  <-> PB2 through D1, C2, R2
 *   - CE   <-> PB4
 *
 * Note I
 *  The approach for using the same MCU pin for both CSN and SCK has been noticed
 *  in the following post:
 *  http://nerdralph.blogspot.ru/2014/01/nrf24l01-control-with-3-attiny85-pins.html
 *
 * Note II
 *   It is possible to wire CE directly to VCC. In this case we have
 *   one more spare MCU pin for fun and profit. However it make sense
 *   to control CE if we care about power consumption: nrf24 consumption
 *   in standby-I (CE = 0) is much less than in standby-II (CE = 1).
 */

void f_csn(int level)
{
	/* proper delays are important for proper driving
	 * of both CSN and SCK by PB2 through D1, R2, C2
	 */
	if (level > 0) {
		PORTB |= (1 << PB2);
		delay_us(64);
	} else {
		PORTB &= ~(1 << PB2);
		delay_us(8);
	}
}

void f_ce(int level)
{
	if (level > 0) {
		PORTB |= (1 << PB4);
	} else {
		PORTB &= ~(1 << PB4);
	}
}

static void spi_pin_init(void)
{
	/* note that PB0, PB1, PB2 are initialized in spi_init */
	DDRB |= (1 << DDB4);
	PORTB &= ~(1 << PB4);

}

uint8_t f_spi_xfer(uint8_t dat)
{
	return spi_fast_shift(dat);
}

/* */

struct rf24 nrf = {
	.delay_us = delay_us,
	.delay_ms = delay_ms,
	.csn = f_csn,
	.ce = f_ce,
	.spi_xfer = f_spi_xfer,
};

/* */

struct rf24 *radio_init(void)
{
	spi_pin_init();
	spi_init();
	spi_set_mode(SPI_MODE_0);

	rf24_init(&nrf);

	return &nrf;
}
