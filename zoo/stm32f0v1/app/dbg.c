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
#include <libopencm3/stm32/usart.h>

#include "delay.h"

#if 0	/* DBG_UART */

int putchar(int c)
{
	uint8_t ch = (uint8_t)c;
	usart_send_blocking(USART1, ch);
	return 0;
}

void dbg_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO14);
	gpio_set_af(GPIOA, GPIO_AF1, GPIO14);

	/* setup USART1 parameters */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_CR2_STOP_1_0BIT);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* enable USART1 */
	usart_enable(USART1);
}

void dbg_blink(int c, int ms_on, int ms_off)
{

}

#else	/* DBG_LED */

int putchar(int c)
{
	return 0;
}

void dbg_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO14);
	gpio_clear(GPIOA, GPIO14);
}

void dbg_blink(int c, int ms_on, int ms_off)
{
	for(int i = 0; i < c; i++) {
		gpio_set(GPIOA, GPIO14);
		delay_ms(ms_on);
		gpio_clear(GPIOA, GPIO14);
		if (i < (c - 1))
			delay_ms(ms_off);
	}
}

#endif
