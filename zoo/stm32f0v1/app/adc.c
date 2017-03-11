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
#include <libopencm3/stm32/adc.h>

#include "delay.h"

/* */

static void rcc_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_ADC);
}

static void adc_init(void)
{
	/* PA0:ADC_IN0, PA1:ADC_IN1, VREF:ADC_IN17 */
	uint8_t channels[] = { 0, 1, ADC_CHANNEL_VREF };

	/* ADC settings */
	adc_power_off(ADC1);
	adc_set_clk_source(ADC1, ADC_CLKSOURCE_ADC);
	adc_calibrate(ADC1);
	adc_set_operation_mode(ADC1, ADC_MODE_SEQUENTIAL);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_enable_temperature_sensor();
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_071DOT5);
	adc_set_regular_sequence(ADC1, sizeof(channels), channels);
	adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);
	adc_disable_analog_watchdog(ADC1);
	adc_enable_vrefint();
	adc_power_on(ADC1);

	/* ADC startup delay */
	delay_ms(100);
}

static void pinmux_init(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);
}

/* */

void adc_volt_init(void)
{
	rcc_init();
	pinmux_init();
	adc_init();
}

void adc_volt_read(int *va, int *vb)
{
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;

	/* conv #1: PA0 */
	adc_start_conversion_regular(ADC1);
	while (!(adc_eoc(ADC1)));
	v1 = adc_read_regular(ADC1);

	/* conv #2: PA1 */
	adc_start_conversion_regular(ADC1);
	while (!(adc_eoc(ADC1)));
	v2 = adc_read_regular(ADC1);

	/* conv #3: VREF ~ ADC CH17 */
	adc_start_conversion_regular(ADC1);
	while (!(adc_eoc(ADC1)));
	v3 = adc_read_regular(ADC1);

	/* STM32F030x4/6/8/C RM0360 Reference manual
	 * Chapter 12.9:
	 * The internal voltage reference (V REFINT ) provides a stable
	 * (bandgap) voltage output for the ADC and Comparators. V
	 * REFINT is internally connected to the ADC_IN17 input channel.
	 * The precise voltage of V REFINT is individually measured for
	 * each part by ST during production test and stored in the
	 * system memory area. It is accessible in read-only mode.
	 *
	 * STM32F030x4/6/8/C datasheet - production data
	 * Chapter 6.3.4:
	 * Vref internal voltage reference: typical 1200mV
	 *
	 * CH17 (internal reference voltage 1200mV) is sampled
	 * in order to calculate the voltage sampled on CH9:
	 * Vin:  CH9 (PB1) input voltage
	 * Vref: C17 (Vref) input voltage
	 *
	 * Vref / v2 ~ mV-per-bit, then
	 * Vin = bits * mV-per-bit = v1 * (Vref / v2)
	 *
	 * In order to carefully handle integer division, the following
	 * calculation is performed:
	 * Vin = (v1 * Vref) / v2
	 *
	 * STM32F030x4/6/8/C datasheet - production data
	 * Chapter 3.10.2:
	 * VREFINT_CAL (2 bytes value) is stored in 0x1FFFF7BA-0x1FFFF7BB
	 *
	 * Note: this calibrated ADC output for Vref can be accessed in
	 * libopencm3 as follows: unsigned int vref = ST_VREFINT_CAL;
	 * FIXME: not sure how we can use it, maybe to introduce
	 * temperature corrections comparing ST_VREFINT_CAL and CH17
	 * readings ?
	 *
	 */

	*va = 1200 * v1 / v3;
	*vb = 1200 * v2 / v3;

	return;
}
