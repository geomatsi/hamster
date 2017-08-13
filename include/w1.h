#ifndef __W1_H__
#define __W1_H__

#include <stdint.h>

/* 1-wire */

struct w1_ops {
	void (*set_pin_high)(void);
	void (*set_pin_low)(void);
	int (*get_pin_val)(void);
	void (*delay_us)(int us);
	void (*delay_ms)(int ms);
};

void w1_register_ops(struct w1_ops *);

int w1_init_transaction(void);
void w1_send_byte(uint8_t);
uint8_t w1_recv_byte(void);

void w1_delay_us(int);
void w1_delay_ms(int);

#endif /* __W1_H__ */
