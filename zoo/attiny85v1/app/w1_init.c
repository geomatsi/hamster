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

#include <avr/io.h>

#include "delay.h"
#include "w1.h"
#include "ds18b20.h"

/* */

static void set_pin_high(void)
{
	DDRB |= _BV(DDB3);
	PORTB |= _BV(PB3);
}

static void set_pin_low(void)
{
	DDRB |= _BV(DDB3);
	PORTB &= ~_BV(PB3);
}

static int get_pin_val(void)
{
	return (PINB & _BV(PB3)) ? 1 : 0;

}

static void set_delay_us(int us)
{
	delay_us(us);
}

static void set_delay_ms(int ms)
{
	delay_ms(ms);
}

static struct w1_ops w1_temp_ops = {
	.set_pin_high	= set_pin_high,
	.set_pin_low	= set_pin_low,
	.get_pin_val	= get_pin_val,
	.delay_us	= set_delay_us,
	.delay_ms	= set_delay_ms,
};

/* */

void w1_temp_init(void)
{
	w1_register_ops(&w1_temp_ops);
	ds18b20_set_res(R12BIT);
}
