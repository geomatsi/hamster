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

#define PB_LIST_LEN 2

bool sensor_encode_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	uint32_t data[PB_LIST_LEN];
	sensor_data sensor = {};
	uint32_t idx;

	data[0] = (uint32_t)read_vcc();
	data[1] = (uint32_t)read_temp_mcp9700();

	/* encode  sensor_data */

	for (idx = 0; idx < PB_LIST_LEN; idx++) {

		printf("protobuf encoding: (%lu, %lu)\n", idx, data[idx]);

		sensor.type = idx;
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

	unsigned int count = 0;

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
		printf("send pkt #%u\n", ++count);
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
			printf("write error: %d\n", ret);
			rf24_flush_tx(nrf);
			rf24_flush_rx(nrf);
		} else {
			printf("written %d bytes\n", pb_len);
		}

		/* enable power-down mode */

		adc_disable();
		power_all_disable();

		wdt_setup(WDTO_8S);
		lpm_bod_off(SLEEP_MODE_PWR_DOWN);

		power_usart0_enable();
		power_adc_enable();
		power_spi_enable();
		adc_enable();

		blink(0, 3, 500);
	}

	return 1;
}
