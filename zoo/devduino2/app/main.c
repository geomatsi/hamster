#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/io.h>

#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include <nRF24L01.h>
#include <RF24.h>

#include "hamster.h"
#include "delay.h"

#include "leds.h"
#include "uart.h"
#include "adc.h"
#include "dht.h"

#include "pb_encode.h"
#include "pb_decode.h"
#include "msg.pb.h"

/* */

extern struct rf24* radio_init(void);
extern long read_temp_mcp9700(void);
extern long read_vcc(void);

/* */

FILE uart_stream = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

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
		WDTCSR |= _BV(WDIE);	\
	} while (0);

ISR(WDT_vect)
{
	wdt_disable();
}

/* */

#define blink(led, count, msec)		\
do {					\
	int c = count;			\
	for (; c > 0; c--) {		\
		led_on(led);		\
		_delay_ms(msec);	\
		led_off(led);		\
		if (c)			\
			_delay_ms(msec);\
	}				\
} while(0);

/* */

#define NUM_CYCLES	3	

bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	sensor_data sensor = {};
	int len = 0;
#define PB_LIST_LEN	4
	uint32_t type[PB_LIST_LEN];
	uint32_t data[PB_LIST_LEN];
	dht_res_t ret;
	long v;
	uint8_t t;
	uint8_t h;
	int cidx;

	cidx = *((int *)*arg);

	switch (cidx) {
	case 0:
#define VBAT_LOW_MV	2750
		v = (long)read_vcc();
		type[len] = (uint32_t)AID_VBAT_LOW;
		data[len++] = (v <  VBAT_LOW_MV) ? 1 : 0;
		break;
	case 1:
		type[len] = (uint32_t)SID_VOLT_MV(0);
		data[len++] = (uint32_t)read_vcc();

		type[len] = (uint32_t)SID_TEMP_C(0);
		data[len++] = (uint32_t)read_temp_mcp9700();
		break;
	case 2:
		dht_power_up();
		_delay_ms(2000);
		ret = dht_read(DHT_TYPE_DHT11, &t, NULL, &h, NULL);
		dht_power_down();
		if (ret) {
			printf("DHT read failure: %u\n", ret);
			type[len] = EID_NODE(0);
			data[len++] = (uint32_t)ret;
		} else {
			type[len] = (uint32_t)SID_TEMP_C(1);
			data[len++] = (uint32_t)t;

			type[len] = (uint32_t)SID_HUM_PER(0);
			data[len++] = (uint32_t)h;
		}

		break;
	default:
		/* should not be here: check NUM_CYCLES */
		return true;
	}

	/* encode  sensor_data */
	for (int idx = 0; idx < len; idx++) {

		printf("protobuf encoding: (%d, %lu)\n", idx, data[idx]);

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
	enum rf24_tx_status ret;
	uint8_t addr[] = NRF_ADDR;
	uint8_t buf[32];
	int cidx = 0;
	
	node_sensor_list message = {};
	pb_ostream_t stream;
	bool pb_status;
	size_t pb_len;

	stdout = &uart_stream;
	stderr = &uart_stream;

	uart_init();

	printf("led_init...\n");
	leds_init();
	blink(0, 5, 100);

	printf("radio_init...\n");
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

	while (1){
		if (++cidx >= NUM_CYCLES)
			cidx = 0;

		printf("cycle #%u\n", cidx);
		memset(buf, 0x0, sizeof(buf));
		stream = pb_ostream_from_buffer(buf, sizeof(buf));

		/* static message part */
		message.node.node = node_id;

		/* repeated message part */
		message.sensor.funcs.encode = &sensor_encode_callback;

		/* pass cycle index as opaque callback data */
		message.sensor.arg = (void *)&cidx;

		pb_status = pb_encode(&stream, node_sensor_list_fields, &message);
		pb_len = stream.bytes_written;

		if (!pb_status) {
			printf("nanopb encoding failed: %s\n", PB_GET_ERROR(&stream));
		} else {
			printf("nanopb encoded %u bytes\n", pb_len);
		}


		ret = rf24_send(nrf, buf, pb_len);
		if (ret != RF24_TX_OK) {
			printf("write error: %d\n", ret);
			rf24_flush_tx(nrf);
			rf24_flush_rx(nrf);
		} else {
			printf("written %d bytes\n", pb_len);
		}

		/* enable power-down mode */

#if 1
		adc_disable();
		power_all_disable();

#define NODE_PERIOD	60

		for(int ts = 0; ts < NODE_PERIOD; ts += 8) {
			wdt_setup(WDTO_8S);
			lpm_bod_off(SLEEP_MODE_PWR_DOWN);
		}

		power_usart0_enable();
		power_adc_enable();
		power_spi_enable();
		adc_enable();
#else
		_delay_ms(1000);
#endif
		blink(0, 3, 500);
	}

	return 1;
}
