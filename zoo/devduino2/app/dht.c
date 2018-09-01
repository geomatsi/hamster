/*
 *
 * Copyright (C) 2016 Jacek Wieczorek
 * Copyright (C) 2018 geomatsi (geomatsi@gmail.com)
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.	See the LICENSE file for details.
 *
 * Based on:
 * - avr-dhtxx at https://github.com/Jacajack/avr-dhtxx
 *
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

#include "dht.h"

#define DHT_TIMEOUT	60

/*
 * dht_ops header should provide the following
 * app-specific defines to control pin:
 *	- DHT_SET_PIN_HIGH()
 *	- DHT_SET_PIN_LOW()
 *	- DHT_PIN_INPUT()
 *	- DHT_GET_PIN_VALUE()
 *	- DHT_PWR_ENABLE()
 *	- DHT_PWR_DISABLE()
 */

#include "dht_ops.h"

void dht_power_up()
{
	DHT_PWR_ENABLE();
}

void dht_power_down()
{
	DHT_PWR_DISABLE();
}

static dht_res_t dht_readb(uint8_t *dest)
{
	/* backup status register */
	uint8_t sreg = SREG;

	uint8_t timeout = 0;
	uint8_t data = 0;
	uint8_t i;

	cli();

	DHT_PIN_INPUT();

	/* read 8 bits */
	for (i = (1 << 7); i; i >>= 1) {
		/* wait for the sync part to finish */
		timeout = 0;
		while (!DHT_GET_PIN_VALUE()) {
			if (timeout++ > DHT_TIMEOUT) {
				SREG = sreg;
				return DHT_ERR_COMM;
			}

			_delay_us(1);
		}

		_delay_us(30);

		if (DHT_GET_PIN_VALUE())
			data |= i;

		/* wait for the data part to finish */
		timeout = 0;
		while (DHT_GET_PIN_VALUE()) {
			if (timeout++ > DHT_TIMEOUT) {
				SREG = sreg;
				return DHT_ERR_COMM;
			}

			_delay_us(1);
		}
	}

	*dest = data;
	SREG = sreg;

	return DHT_OK;
}

dht_res_t dht_read(dht_type_t type, uint8_t *t_int, uint8_t *t_dec, uint8_t *h_int, uint8_t *h_dec)
{
	/* status register backup */
	uint8_t sreg = SREG;

	uint8_t csum = 0;
	uint8_t data[5];
	dht_res_t ret;
	uint8_t i;

	if ((type != DHT_TYPE_DHT11) && (type != DHT_TYPE_DHT22))
		return DHT_ERR_SUPP;

	/* send start signal */
	DHT_SET_PIN_LOW();

	/* adjust start signal time for DHT11/DHT22 */

	if (type == DHT_TYPE_DHT22)
		_delay_us(500);
	else
		_delay_ms(18);

	cli();

	/* turn pin into input and wait for acknowledgement */
	DHT_PIN_INPUT();
	_delay_us(30 + 40);

	/* comm proto check #1 */
	if (DHT_GET_PIN_VALUE()) {
		SREG = sreg;
		return DHT_ERR_COMM;
	}

	_delay_us(80);

	/* comm proto check #2 */
	if (!DHT_GET_PIN_VALUE()) {
		SREG = sreg;
		return DHT_ERR_COMM;
	}

	/* restore status */
	SREG = sreg;
	_delay_us(40);

	/* read data bytes */
	for (i = 0; i < 5; i++) {
		ret = dht_readb(&data[i]);
		if (ret)
			return ret;
	}

	/* calculate checksum */
	for (i = 0; i < 4; i++)
		csum += data[i];

	if (csum != data[4])
		return DHT_ERR_CSUM;

	/* Note: for DHT11 decimal parts are zeros */

	if (h_int)
		*h_int = data[0];

	if (h_dec)
		*h_dec = data[1];

	if (t_int)
		*t_int = data[2];

	if (t_dec)
		*t_dec = data[3];

	return DHT_OK;
}
