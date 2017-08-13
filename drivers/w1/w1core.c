/* simple blocking 1-wire implementation */

#include "w1.h"

static struct w1_ops *w1;

/* register ops */

void w1_register_ops(struct w1_ops *ops)
{
	w1 = ops;
}

/* 1wire reset and check presence */

int w1_init_transaction(void)
{
	uint8_t val;

	/* 1wire reset */
	w1->set_pin_low();
	w1->delay_us(600);

	/* 1wire relax */
	w1->set_pin_high();
	w1->delay_us(80);

	/* 1wire check presence */
	val = w1->get_pin_val();
	w1->delay_us(520);

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
		w1->set_pin_low();

		if (bit) {
			w1->delay_us(2);
			w1->set_pin_high();
			w1->delay_us(58);
		} else {
			w1->delay_us(60);
			w1->set_pin_high();
		}

		/* min here is 1 usec */
		w1->delay_us(5);
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

		w1->set_pin_low();
		w1->delay_us(3);
		w1->set_pin_high();
		w1->delay_us(8);

		bit = w1->get_pin_val();
		w1->delay_us(40);

		if (bit) {
			byte |= (0x1 << i);
		}

		w1->delay_us(5);
	}

	return byte;
}

void w1_delay_us(int us)
{
	w1->delay_us(us);
}

void w1_delay_ms(int ms)
{
	w1->delay_ms(ms);
}
