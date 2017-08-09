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

#include <RF24.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "msg.pb.h"

#include "hamster.h"
#include "hc-sr04.h"
#include "delay.h"

/* FIXME: create tinylib and its header for this stuff */

#define size_t  unsigned int
extern int sprintf(char *out, const char *format, ...);
extern int printf(const char *format, ...);
extern void *memset(void *s, int c, size_t n);

/* */

struct rf24 *radio_init(void);
void stdout_init(void);

void w1_temp_init(void);
int w1_temp_read(void);

void adc_volt_init(void);
void adc_volt_read(int *va, int *vb);

/* */

#define PB_LIST_LEN	3

static uint32_t count = 0;
static uint32_t range = 0;
static int temp = 0;
static int va = 0;
static int vb = 0;

bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	uint32_t type[PB_LIST_LEN];
	uint32_t data[PB_LIST_LEN];
	sensor_data sensor = {};
	int len = 0;

	if (va > 3000) {
		type[len] = (uint32_t)AID_WATER_LVL;
		data[len++] = (uint32_t)1;
	} else {
		type[len] = (uint32_t)SID_TEMP_C;
		data[len++] = (uint32_t)temp;

		type[len] = (uint32_t)SID_VOLT_MV;
		data[len++] = (uint32_t)vb;

		if (hc_sr04_valid_range(range)) {
			type[len] = (uint32_t)SID_RANGE_MM;
			data[len++] = (uint32_t)range;
		} else {
			/* report range sensor failure */
			type[len] = (uint32_t)AID_NODE_ERR;
			data[len++] = (uint32_t)SID_RANGE_MM;
		}
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

int main(void)
{

#if defined(NODE_ID)
	uint32_t node_id = NODE_ID;
#else
#error "NODE_ID is not defined"
#endif

	struct rf24 *nrf;
	uint8_t buf[32];

	node_sensor_list message = node_sensor_list_init_default;
	pb_ostream_t stream;
	bool pb_status;
	size_t pb_len;

	enum rf24_tx_status ret;

	rcc_clock_setup_in_hsi_out_48mhz();

	delay_init();
	stdout_init();
	w1_temp_init();
	adc_volt_init();
	hc_sr04_init(48 /* MHz */);

	nrf = radio_init();

	printf("start xmit cycle...\r\n");
	while (1) {

		/* read sensors */

		/* temperature: ds18b20 */
		temp = w1_temp_read();

		/* VBAT and light: stm32f0 ADC */
		adc_volt_read(&va, &vb);

		/* range: hc-sr04 */
		hc_sr04_setup_echo_capture();
		hc_sr04_trigger_pulse();
		range = hc_sr04_get_range();

		printf("t[%d] va[%d] vb[%d] r[%u]\n",
			temp, va, vb, (unsigned int)range);

		/* send data */

		printf("pkt #%u\n", (unsigned int)++count);
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
		} else {
			printf("written %d bytes\n", pb_len);
		}

		delay_ms(1000);
	}

	return 0;
}
