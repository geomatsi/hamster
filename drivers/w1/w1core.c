/* simple blocking 1-wire implementation */

#include <delay.h>

/*
 * w1_ops header should provide the following
 * app-specific defines to control pin:
 *	- PLT_SET_PIN_HIGH()
 *	- PLT_SET_PIN_LOW()
 *	- PLT_GET_PIN_VALUE()
 *	- PLT_DELAY_US()
 */

#include "w1_ops.h"

/* 1wire reset and check presence */

int w1_init_transaction(void)
{
	uint8_t val;

	/* 1wire reset */
	PLT_SET_PIN_LOW();
	PLT_DELAY_US(600);

	/* 1wire relax */
	PLT_SET_PIN_HIGH();
	PLT_DELAY_US(80);

	/* 1wire check presence */
	val = PLT_GET_PIN_VALUE();
	PLT_DELAY_US(520);

	return val ? 0 : 1;
}

/* send 1wire data to device */

void w1_send_byte(uint8_t byte)
{
	volatile uint8_t bit;
	int i;

	for(i = 0; i < 8; i++) {

		/* NB !!!
		 * Here bit must be declared as volatile. Otherwise compiler
		 * may reorder code and perform shift after pin is set low.
		 * This reordering may result in significant increase
		 * of low pulse length on 1MHz devices.
		 */

		bit = (byte >> i) & 0x01;
		PLT_SET_PIN_LOW();

		if (bit) {
			PLT_DELAY_US(2);
			PLT_SET_PIN_HIGH();
			PLT_DELAY_US(58);
		} else {
			PLT_DELAY_US(60);
			PLT_SET_PIN_HIGH();
		}

		/* min here is 1 usec */
		PLT_DELAY_US(5);
	}
}

/* recv 1wire data from device */

uint8_t w1_recv_byte(void)
{
	volatile uint8_t byte;
	volatile uint8_t bit;
	int i;

	byte = 0;

	for (i = 0; i < 8; i++) {

		PLT_SET_PIN_LOW();
		PLT_DELAY_US(3);
		PLT_SET_PIN_HIGH();
		PLT_DELAY_US(8);

		bit = PLT_GET_PIN_VALUE();
		PLT_DELAY_US(40);

		if (bit) {
			byte |= (0x1 << i);
		}

		PLT_DELAY_US(5);
	}

	return byte;
}

void w1_delay_us(int us)
{
	PLT_DELAY_US(us);
}

void w1_delay_ms(int ms)
{
	PLT_DELAY_MS(ms);
}
