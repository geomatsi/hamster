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

/*
 * stm32/f0: blocking delay API
 *   - simple implementation of usec and msec blocking delays using TIM14 timer
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "delay.h"

static volatile uint32_t ready = 0;

void tim14_isr(void)
{
	ready = 1;

	/* clear interrrupt flag */
	TIM_SR(TIM14) &= ~TIM_SR_UIF;

	/* stop counter */
	TIM_CR1(TIM14) &= ~TIM_CR1_CEN;
}

void tim_prepare(uint16_t psc, uint16_t arr)
{
	/*
	 * TIM14: simple upcouning mode
	 */

	/* set timer start value */
	TIM_CNT(TIM14) = 0;

	/* set timer prescaler
	 *   - TIM_PSC = (48 - 1) : 48_000_000 Hz / 48 = 1_000_000 ticks per second
	 *   - TIM_ARR = N : irq is generated after N usecs
	 */
	TIM_PSC(TIM14) = psc;

	/* end timer value:  this is reached =>  interrupt is generated */
	TIM_ARR(TIM14) = arr;

	/* update interrupt enable */
	TIM_DIER(TIM14) = TIM_DIER_UIE;

	/* start counter */
	TIM_CR1(TIM14) = TIM_CR1_URS | TIM_CR1_CEN;

	/* force update generation */
	TIM_EGR(TIM14) = TIM_EGR_UG;
}

void delay_init(void)
{
	rcc_periph_clock_enable(RCC_TIM14);

	/* TIM14: setup interrupt */
	nvic_enable_irq(NVIC_TIM14_IRQ);
	nvic_set_priority(NVIC_TIM14_IRQ, 1);
}

/* FIXME:
 *   - need properly handle 16bit overflows
 *   - current range: 0 <= msec <= 65535
 *   - current range: 0 <= usec <= 65535
 */
void delay_ms(int msec)
{
	ready = 0;
	tim_prepare(48000 - 1, (uint16_t)msec);
	while(!ready);
}

void delay_us(int usec)
{
	ready = 0;
	tim_prepare(48 - 1, (uint16_t)usec);
	while(!ready);
}
