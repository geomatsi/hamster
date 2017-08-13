/*
 * Based on stm32f1 time interrupt example from libopencm3-example project
 *
 * Copyright (C) 2010 Thomas Otto <tommi@viadmin.org>
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

/* Example: TIM pulse capture to support HC-SR04 ultrasonic range sensor */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

/* capture logic */

enum capture_state {
	READY,
	RUNNING,
	DONE,
	ERROR,
};

static volatile enum capture_state state = READY;
static volatile uint32_t pulse = 0;
static volatile uint32_t freq = 48;

void tim1_cc_isr(void)
{
	volatile uint16_t status = TIM_SR(TIM1);


	if (status & (TIM_SR_CC2OF | TIM_SR_UIF)) {
		state = ERROR;
		goto irq_done;
	}

	switch (state) {
	case READY:
		if (status & TIM_SR_CC2IF) {
			/* first pulse edge: start counter */
			TIM_CR1(TIM1) |= TIM_CR1_CEN;

			state = RUNNING;
		} else {
			/* unexpected interrupt */
			state = ERROR;
		}

		break;
	case RUNNING:
		if (status & TIM_SR_CC2IF) {
			/* second pulse edge: stop counter*/
			TIM_CR1(TIM1) &= ~TIM_CR1_CEN;

			/* read captured counter */
			pulse = TIM_CCR2(TIM1);

			/* disable interrupts */
			TIM_DIER(TIM1) = 0x0;

			state = DONE;
		} else {
			/* unexpected interrupt */
			state = ERROR;
		}

		break;
	case DONE:
		/* unexpected interrupt */
		state = ERROR;
		break;
	default:
		break;
	}

irq_done:

	/* error: stop counter and disable interrupts */
	if (state == ERROR) {
		/* disable interrupts */
		TIM_DIER(TIM1) = 0x0;
	}

	/* clear interrrupt flags */
	TIM_SR(TIM1) = 0x0;
}

/* */

void hc_sr04_setup_echo_capture()
{
	/*
	 * TIM1: input capture mode
	 */

	/* timer prescaler for freq MHz CPU clock: 1000000 counts per second (1uS tick) */
	TIM_PSC(TIM1) = freq;

	/* TIM1 mode: input capture mode */
	TIM_CCMR1(TIM1) =
		TIM_CCMR1_CC2S_IN_TI2 |		/* CC1 is input, IC1 is mapped to TI2 */
		TIM_CCMR1_IC2F_CK_INT_N_8 |	/* input filter: 8 clock cycles */
		TIM_CCMR1_IC2PSC_OFF;		/* capture is done on each edge */

	/* enable capture: sensitive to both edges */
	TIM_CCER(TIM1) = TIM_CCER_CC2E | TIM_CCER_CC2P | TIM_CCER_CC2NP;

	/* enable interrupts */
	TIM_DIER(TIM1) = TIM_DIER_CC2IE | TIM_DIER_UIE;

	/* only counter overflow should generate UEV */
	TIM_CR1(TIM1) &= ~TIM_CR1_UDIS;
	TIM_CR1(TIM1) |= TIM_CR1_URS;

	/* force update generation */
	TIM_EGR(TIM1) |= TIM_EGR_UG;

	/* DO NOT start counter on setup */
	TIM_CR1(TIM1) &= ~TIM_CR1_CEN;

	/* clear interrrupt flags */
	TIM_SR(TIM1) = 0x0;

	/*
	 * Setup capture FSM
	 */

	state = READY;
	pulse = 0;
}

void hc_sr04_trigger_pulse(void)
{
	volatile int i;

	/* TRIG of SR04 must receive a pulse at least 10 usec */
	gpio_set(GPIOA, GPIO10);

	/* ~50 usec for clk 48MHz */
	for (i = 0; i < 150; i++)
		__asm__("nop");

	gpio_clear(GPIOA, GPIO10);
}

uint32_t hc_sr04_get_range(void)
{
	volatile uint32_t timeout = 0;

	while ((state == READY) || (state == RUNNING)) {
		if (timeout++ == 0xfffff) {
			/* TODO: reset hc_sr04 after failure */
			return 0xffff;
		}
	}

	if (state == ERROR) {
		/* TODO: reset hc_sr04 after failure */
		return 0xffff;
	}

	return pulse / 58;
}

int hc_sr04_valid_range(uint32_t range)
{
	return ((0 <= range) && range <= 1000) ? 1 : 0;
}

void hc_sr04_init(uint32_t mhz)
{
	freq = mhz;

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_TIM1);

	/* PA9 5V tolerant: echo pin as TIM1 CH2 input */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOA, GPIO_AF2, GPIO9);

	/* PA10 5V tolerant: trigger pin */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);

	/* TIM1 interrupt */
	nvic_enable_irq(NVIC_TIM1_CC_IRQ);
	nvic_set_priority(NVIC_TIM1_CC_IRQ, 1);
}
