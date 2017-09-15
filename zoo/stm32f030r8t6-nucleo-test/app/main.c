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
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>

#include <RF24.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "msg.pb.h"

#include "hamster.h"
#include "delay.h"

#include "rtc-utils.h"

/* */

/* FIXME: create tinylib and its header for this stuff */

#define size_t  unsigned int
extern int printf(const char *format, ...);
extern void *memset(void *s, int c, size_t n);

/* */

struct rf24 *radio_init(void);

/* */

#define PB_LIST_LEN	5

#if defined(NODE_ID)
uint32_t node_id = NODE_ID;
#else
#error "NODE_ID is not defined"
#endif

static uint32_t count = 1;

static int va = 1;
static int vb = 2;
static int vc = 3;

bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	uint32_t type[PB_LIST_LEN];
	uint32_t data[PB_LIST_LEN];
	sensor_data sensor = {};
	int len = 0;

	type[len] = (uint32_t)AID_VBAT_LOW;
	data[len++] = count;

	type[len] = (uint32_t)SID_TEMP_C(0);
	data[len++] = (uint32_t)va;

	type[len] = (uint32_t)SID_VOLT_MV(0);
	data[len++] = (uint32_t)vb;

	type[len] = (uint32_t)SID_RANGE_SM(0);
	data[len++] = (uint32_t)vc;

	/* encode  sensor_data */

	for (int idx = 0; idx < len; idx++) {
		sensor.type = type[idx];
		sensor.data = data[idx];

		if (!pb_encode_tag_for_field(stream, field)) {
			printf("protobuf tag encoding failed: %s\n", PB_GET_ERROR(stream));
			return false;
		}

		if (!pb_encode_submessage(stream, sensor_data_fields, &sensor)) {
			printf("protobuf submessage encoding failed: %s\n", PB_GET_ERROR(stream));
			return false;
		}
	};

	return true;
}

/* */

int putchar(int c)
{
	uint8_t ch = (uint8_t)c;
	usart_send_blocking(USART1, ch);
	return 0;
}

void uart_init(void)
{
	/* USART1 init */

	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART1);

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_set_af(GPIOB, GPIO_AF0, GPIO6);

	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_CR2_STOP_1_0BIT);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_enable(USART1);
}
/* */

void re_init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
}

static inline __attribute__((always_inline)) void __WFI(void)
{
	__asm volatile ("wfi");
}

void scb_enable_deep_sleep_mode(void)
{
	SCB_SCR |= SCB_SCR_SLEEPDEEP;
}

/* */

int main(void)
{
	struct rf24 *nrf;
	uint8_t buf[32];

	node_sensor_list message = node_sensor_list_init_default;
	pb_ostream_t stream;
	bool pb_status;
	size_t pb_len;

	enum rf24_tx_status ret;

	rcc_clock_setup_in_hsi_out_48mhz();

	/* delay: init first, others may need delay_ms/delay_us */
	delay_init();
	rtc_setup();
	uart_init();

	nrf = radio_init();
	rf24_print_status(nrf);

	/* */

	printf("start xmit cycle...\r\n");
	while (1) {

		printf("pkt #%u\n", (unsigned int)count++);

		/* send data */

		memset(buf, 0x0, sizeof(buf));
		stream = pb_ostream_from_buffer(buf, sizeof(buf));

		/* static message part */
		message.node.node = node_id;

		/* repeated message part */
		message.sensor.funcs.encode = &sensor_encode_callback;

		pb_status = pb_encode(&stream, node_sensor_list_fields, &message);
		pb_len = stream.bytes_written;

		if (!pb_status) {
			printf("nanopb encoding failed: %s\r\n", PB_GET_ERROR(&stream));
		} else {
			printf("nanopb encoded %u bytes\r\n", pb_len);
		}

		ret = rf24_send(nrf, buf, pb_len);
		if (ret != RF24_TX_OK) {
			printf("send error: %d\r\n", ret);
			rf24_flush_tx(nrf);
			rf24_flush_rx(nrf);
		} else {
			printf("written %d bytes\r\n", pb_len);
		}

		/* enter low-power mode: stop mode */
		configure_next_alarm(0, 10);
		scb_enable_deep_sleep_mode();
		pwr_set_stop_mode();
		pwr_voltage_regulator_low_power_in_stop();
		pwr_clear_wakeup_flag();
		__WFI();

		/* re-init after low-power mode */
		re_init();
	}

	return 0;
}
