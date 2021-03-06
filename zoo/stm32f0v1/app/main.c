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
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/iwdg.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>

#include <RF24.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "msg.pb.h"

#include "hamster.h"
#include "hc-sr04.h"
#include "ds18b20.h"
#include "delay.h"

#include "rtc-utils.h"

/* FIXME: create tinylib and its header for this stuff */

#define size_t  unsigned int
extern int sprintf(char *out, const char *format, ...);
extern int printf(const char *format, ...);
extern void *memset(void *s, int c, size_t n);

/* */

struct rf24 *radio_init(void);
void dbg_init(void);
void dbg_blink(int c, int ms_on, int ms_off);

#if defined(NODE_TEMP)
void w1_temp_init(void);
static int temp = 0;
#endif

#if defined(NODE_ADC)
void adc_volt_init(void);
void adc_volt_read(int *va, int *vb);
static int va = 0;
static int vb = 0;
#endif

#if defined(NODE_RANGE)
static uint32_t range = 0;
#endif

/* */

#define PB_LIST_LEN	5

#if defined(NODE_ID)
uint32_t node_id = NODE_ID;
#else
#error "NODE_ID is not defined"
#endif

static uint32_t count = 0;

bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	uint32_t type[PB_LIST_LEN];
	uint32_t data[PB_LIST_LEN];
	sensor_data sensor = {};
	int len = 0;

	/* message circle: alerts -> sensor1 -> ... -> sensorN */

	switch (count & 0x3) {
		case 0x0:
			/* report count: diag(0) */
			type[len] = (uint32_t)EID_NODE(0);
			data[len++] = (uint32_t)count;
			break;
		case 0x1:
#if defined(NODE_TEMP)
			if (ds18b20_valid_temp(temp)) {
				type[len] = (uint32_t)SID_TEMP_C(0);
				data[len++] = (uint32_t)temp;
			} else {
				/* report temp sensor failure */
				type[len] = (uint32_t)AID_NODE_ERR;
				data[len++] = (uint32_t)SID_TEMP_C(0);
			}
#else
			/* no temp sensor: report diag(1) */
			type[len] = (uint32_t)EID_NODE(1);
			data[len++] = (uint32_t)SID_TEMP_C(0);
#endif
			break;
		case 0x2:
#if defined(NODE_RANGE)
			if (hc_sr04_valid_range(range)) {
				type[len] = (uint32_t)SID_RANGE_SM(0);
				data[len++] = (uint32_t)range;
			} else {
				/* report range sensor failure */
				type[len] = (uint32_t)AID_NODE_ERR;
				data[len++] = (uint32_t)SID_RANGE_SM(0);
			}
#else
			/* no range sensor: report diag(1) */
			type[len] = (uint32_t)EID_NODE(1);
			data[len++] = (uint32_t)SID_RANGE_SM(0);
#endif
			break;
		case 0x3:
#if defined(NODE_ADC)
			type[len] = (uint32_t)SID_VOLT_MV(0);
			data[len++] = (uint32_t)vb;
#else
			/* no range sensor: report diag(1) */
			type[len] = (uint32_t)EID_NODE(1);
			data[len++] = (uint32_t)SID_VOLT_MV(0);
#endif
			break;
		default:
			/* should not be here: report event(0x111) */
			type[len] = (uint32_t)EID_NODE(0x11);
			data[len++] = (uint32_t)__LINE__;
			break;
	}

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

void node_read_sensors(void)
{
	/* temperature: ds18b20 sensor */
#if defined(NODE_TEMP)
	temp = ds18b20_read_temp();
	printf("temp [%u]\r\n", (unsigned int)temp);
#endif

	/* VBAT and water level trigger: ADC */
#if defined(NODE_ADC)
	adc_volt_read(&va, &vb);

	/* VBAT = vb * 3 */
	vb = vb * 3;
#endif

	/* range: hc-sr04 sensor */
#if defined(NODE_RANGE)
	hc_sr04_setup_echo_capture();
	hc_sr04_trigger_pulse();
	range = hc_sr04_get_range();
	printf("range [%u]\r\n", (unsigned int)range);
#endif
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
	dbg_init();

#if defined(NODE_LPR)
	rtc_setup();
#endif

#if defined(NODE_TEMP)
	w1_temp_init();
#endif

#if defined(NODE_ADC)
	adc_volt_init();
#endif

#if defined(NODE_RANGE)
	hc_sr04_init(48 /* MHz */);
#endif

	nrf = radio_init();

	dbg_blink(3, 1000, 100);

	iwdg_set_period_ms(30000);
	iwdg_start();

	/* */

	printf("start xmit cycle...\r\n");
	while (1) {

		iwdg_reset();
		printf("pkt #%u\n", (unsigned int)count++);
		dbg_blink(2, 500, 100);

		node_read_sensors();

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
			printf("nanopb encoding failed: %s\n", PB_GET_ERROR(&stream));
		} else {
			printf("nanopb encoded %u bytes\n", pb_len);
		}

		ret = rf24_send(nrf, buf, pb_len);
		if (ret != RF24_TX_OK) {
			printf("send error: %d\n", ret);
			rf24_flush_tx(nrf);
			rf24_flush_rx(nrf);
			dbg_blink(5, 100, 100);
		} else {
			printf("written %d bytes\n", pb_len);
			dbg_blink(2, 100, 100);
		}

#if defined(NODE_LPR)
		/* enter low-power mode: stop mode */
		configure_next_alarm(0, 30);
		scb_enable_deep_sleep_mode();
		pwr_set_stop_mode();
		pwr_voltage_regulator_low_power_in_stop();
		pwr_clear_wakeup_flag();
		__WFI();

		/* re-init after low-power mode */
		re_init();
#else
		iwdg_reset();
		delay_ms(10000);
#endif
	}

	return 0;
}
