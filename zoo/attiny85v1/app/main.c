#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#include "delay.h"

#include "pb_encode.h"
#include "pb_decode.h"
#include "msg.pb.h"

#include "hamster.h"
#include "ds18b20.h"
#include "adc.h"
#include "radio.h"

/* */

#define	lpm_bod_off(mode)			\
	do {					\
		set_sleep_mode(mode);		\
		cli();				\
		sleep_enable();			\
		sleep_bod_disable();		\
		sei();				\
		sleep_cpu();			\
		sleep_disable();		\
		sei();				\
	} while (0);

#define wdt_setup(period)		\
	do {				\
		wdt_enable(period);	\
		WDTCR |= _BV(WDIE);	\
	} while (0);

ISR(WDT_vect)
{
	wdt_disable();
}

/* */

#define VBAT_LOW_MV	2500
#define PB_LIST_LEN	5

static unsigned int count = 0;
uint32_t vbat = 0;
uint32_t temp = 0;

/* */

static bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	uint32_t type[PB_LIST_LEN];
	uint32_t data[PB_LIST_LEN];
	sensor_data sensor = {};
	uint32_t len = 0;

	if (count & 1) {
		type[len] = (uint32_t)AID_VBAT_LOW;
		data[len++] = (vbat <  VBAT_LOW_MV) ? 1 : 0;
	} else {
		type[len] = SID_VOLT_MV(0);
		data[len++] = vbat;

		type[len] = SID_TEMP_C(0);
		data[len++] = temp;
	}

	/* encode sensor_data */

	for (int idx = 0; idx < len; idx++) {

		sensor.type = type[idx];
		sensor.data = data[idx];

		if (!pb_encode_tag_for_field(stream, field)) {
			return false;
		}

		if (!pb_encode_submessage(stream, sensor_data_fields, &sensor)) {
			return false;
		}
	};

	return true;
}

/* read battery */

uint32_t get_battery_voltage(void)
{
	uint32_t v;

	/* re-init adc: Vref = Vcc ?= 3v3, Vbg (1v1) input channel */
	adc_scm_init(0, 12);
	delay_ms(5);

	/* read twice to make sure ADC is settled */
	v = (uint32_t)adc_scm_read();
	delay_ms(5);
	v = (uint32_t)adc_scm_read();

	/* Vcc = 1024 * 1100 / ADC */
	v = 1126400 / v;

	return v;
}

/* main */

int main (void)
{
#if defined(NODE_ID)
	uint32_t node_id = NODE_ID;
#else
#error "NODE_ID is not defined"
#endif

	uint8_t addr[] = NRF_ADDR;
	uint8_t buf[32];

	enum rf24_tx_status ret;
	struct rf24 *nrf;

	node_sensor_list message = {};
	pb_ostream_t stream;
	bool pb_status;
	size_t pb_len;

	/* */

	ds18b20_set_res(R12BIT);
	nrf = radio_init();

	delay_ms(500);

	rf24_enable_dyn_payload(nrf);
	rf24_set_retries(nrf, 0xf /* retry delay 4000us */, 5 /* retries */);

	rf24_set_channel(nrf, NRF_CHAN);
	rf24_set_data_rate(nrf, NRF_RATE);
	rf24_set_crc_mode(nrf, NRF_CRC);
	rf24_set_pa_level(nrf, NRF_PA);

	rf24_setup_ptx(nrf, addr);
	rf24_start_ptx(nrf);

	while (1) {

		/* get new measurements */
		vbat = get_battery_voltage();
		temp = ds18b20_read_temp();

		memset(buf, 0x0, sizeof(buf));
		stream = pb_ostream_from_buffer(buf, sizeof(buf));

		/* static message part */
		message.node.node = node_id;

		/* repeated message part */
		message.sensor.funcs.encode = &sensor_encode_callback;

		pb_status = pb_encode(&stream, node_sensor_list_fields, &message);
		pb_len = stream.bytes_written;

		if (!pb_status) {
			/* FIXME */
		}

		ret = rf24_send(nrf, buf, pb_len);
		if (ret != RF24_TX_OK) {
			/* send error */
			rf24_flush_tx(nrf);
			rf24_flush_rx(nrf);
		}

		/* enable power-down mode */

		adc_disable();
		power_all_disable();

#define NODE_PERIOD	300

		for(int ts = 0; ts < NODE_PERIOD; ts += 8) {
			wdt_setup(WDTO_8S);
			lpm_bod_off(SLEEP_MODE_PWR_DOWN);
		}

		power_adc_enable();
		power_usi_enable();
		adc_enable();
	}

	return 1;
}
