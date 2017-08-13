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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "delay.h"

#include "w1.h"
#include "ds18b20.h"


/* */

static void set_pin_high(void)
{
	gpio_set(GPIOB, GPIO1);
}

static void set_pin_low(void)
{
	gpio_clear(GPIOB, GPIO1);
}

static int get_pin_val(void)
{
	return gpio_get(GPIOB, GPIO1) ? 1 : 0;
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
	rcc_periph_clock_enable(RCC_GPIOB);

	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO1);

	w1_register_ops(&w1_temp_ops);
	ds18b20_set_res(R12BIT);
}
