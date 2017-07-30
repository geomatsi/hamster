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
#include <libopencm3/stm32/spi.h>

#include <RF24.h>

#include "delay.h"

/* */

static void rcc_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_SPI1);
}

static void spi_init(void)
{
	spi_set_master_mode(SPI1);
	spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_16);
	spi_set_clock_polarity_0(SPI1);
	spi_set_clock_phase_0(SPI1);
	spi_set_full_duplex_mode(SPI1);
	spi_set_unidirectional_mode(SPI1);
	spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
	spi_enable_software_slave_management(SPI1);
	spi_send_msb_first(SPI1);
	spi_set_nss_high(SPI1);
	spi_disable_crc(SPI1);
	spi_fifo_reception_threshold_8bit(SPI1);
	spi_i2s_mode_spi_mode(SPI1);
	spi_enable(SPI1);
}

static void pinmux_init(void)
{
	/* NRF24: SPI1 pins - MOSI/MISO/SCK */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,  GPIO5 | GPIO6 | GPIO7);
	gpio_set_af(GPIOA, GPIO_AF0, GPIO5 | GPIO6 | GPIO7);

	/* NRF24: CE pin */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,  GPIO2);

	/* NRF24: CS pin */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO3);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,  GPIO3);
}

/* */

static void f_csn(int level)
{
	(level > 0) ? gpio_set(GPIOA, GPIO3) : gpio_clear(GPIOA, GPIO3);
}

static void f_ce(int level)
{
	(level > 0) ? gpio_set(GPIOA, GPIO2) : gpio_clear(GPIOA, GPIO2);
}

static uint8_t f_spi_xfer(uint8_t dat)
{
	spi_send8(SPI1, dat);
	return spi_read8(SPI1);
}

struct rf24 nrf24_ops = {
	.delay_us = delay_us,
	.delay_ms = delay_ms,
	.csn = f_csn,
	.ce = f_ce,
	.spi_xfer = f_spi_xfer,
};

/* */

struct rf24 *radio_init(void)
{
	uint8_t addr[] = {'E', 'F', 'C', 'L', 'I'};
	struct rf24 *nrf = &nrf24_ops;

	/* configure stm32/f0 peripherals */
	rcc_init();
	pinmux_init();
	spi_init();

	/* init nRF24 library */
	rf24_init(&nrf24_ops);

	/* configure RX settings */
	rf24_enable_dyn_payload(nrf);
	rf24_set_retries(nrf, 0xf /* retry delay 4000us */, 10 /* retries */);
	rf24_set_channel(nrf, 10);
	rf24_set_data_rate(nrf, RF24_RATE_250K);
	rf24_set_crc_mode(nrf, RF24_CRC_16_BITS);
	rf24_set_pa_level(nrf, RF24_PA_MIN);

	rf24_setup_ptx(nrf, addr);
	rf24_start_ptx(nrf);

	return nrf;
}
