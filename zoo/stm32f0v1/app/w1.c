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

static void rcc_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOB);
}

static void pinmux_init(void)
{
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO1);
}

/* */

void w1_temp_init(void)
{
	rcc_init();
	pinmux_init();

	ds18b20_set_res(R12BIT);
}

int32_t w1_temp_read(void)
{
	uint8_t data[9];
	int32_t temp;
	uint8_t i;

	/* reset and check presence */
	if (!w1_init_transaction())
		return 1000;

	/* skip ROM: next command can be broadcasted */
	w1_send_byte(SKIP_ROM);

	/* start single temperature conversion */
	w1_send_byte(CONVERT_T);

	/* temperature conversion takes ~1sec */
	delay_ms(1000);

	/* reset and check presence */
	if (!w1_init_transaction())
		return 2000;

	/* skip ROM: careful !!! works only for one device on bus: next command is unicast */
	w1_send_byte(SKIP_ROM);

	/* read scratchpad */
	w1_send_byte(READ_PAD);

	/* get all scratchpad bytes */
	for (i = 0; i < 9; i++)
		data[i] = w1_recv_byte();

	/* check crc */
	if (!ds18b20_crc_check(data, 9))
		return 3000;

	/* calculate temperature */
	temp = ds18b20_get_temp(data[1], data[0]);

	return temp;
}
