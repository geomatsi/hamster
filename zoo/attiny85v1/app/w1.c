/*
 * Copyright (C) 2017 Sergey Matyukevich <geomatsi@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "delay.h"
#include "w1.h"

/* */

#define SET_PIN_HIGH()			\
	do {				\
		DDRB |= _BV(DDB3);	\
		PORTB |= _BV(PB3);	\
	} while (0)

#define SET_PIN_LOW()			\
	do {				\
		DDRB |= _BV(DDB3);	\
		PORTB &= ~_BV(PB3);	\
	} while (0)

#define GET_PIN_VALUE()	(PINB & _BV(PB3))

/* */

int w1_init_transaction(void)
{
	uint8_t val;

	cli();

	SET_PIN_LOW();
	_delay_us(600);

	SET_PIN_HIGH();
	_delay_us(80);

	val = GET_PIN_VALUE();
	_delay_us(520);

	sei();

	return val ? 0 : 1;
}

void w1_send_byte(uint8_t byte)
{
	volatile uint8_t bit;
	int i;

	cli();

	for(i = 0; i < 8; i++) {

		/* bit is declared as volatile: otherwise compiler may reorder
		 * code and perform shift _after_ pin is set low, which may
		 * significantly increase low pulse on 1MHz devices
		 */

		bit = (byte >> i) & 0x01;
		SET_PIN_LOW();

		if (bit) {
			_delay_us(2);
			SET_PIN_HIGH();
			_delay_us(58);
		} else {
			_delay_us(60);
			SET_PIN_HIGH();
		}

		_delay_us(5);
	}

	sei();
}

uint8_t w1_recv_byte(void)
{
	volatile uint8_t byte;
	volatile uint8_t bit;
	int i;

	byte = 0;

	cli();

	for (i = 0; i < 8; i++) {

		SET_PIN_LOW();
		_delay_us(3);
		SET_PIN_HIGH();
		_delay_us(8);

		bit = GET_PIN_VALUE();
		_delay_us(40);

		if (bit) {
			byte |= (0x1 << i);
		}

		_delay_us(5);
	}

	sei();

	return byte;
}

void w1_delay_ms(int ms)
{
	delay_ms(ms);
}
