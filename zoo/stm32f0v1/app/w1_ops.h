#ifdef __W1_OPS_H__
#error "w1 ops has been already defined"
#else
#define __W1_OPS_H__

#include <libopencm3/stm32/gpio.h>

/* platform- and app- specific 1-wire pin control */

#define PLT_SET_PIN_HIGH()				\
	do {					\
		gpio_set(GPIOB, GPIO1);		\
	} while (0)

#define PLT_SET_PIN_LOW()				\
	do {					\
		gpio_clear(GPIOB, GPIO1);	\
	} while (0)

#define PLT_GET_PIN_VALUE()	(gpio_get(GPIOB, GPIO1) ? 1 : 0)

#define PLT_DELAY_US(t)	(delay_us(t))
#define PLT_DELAY_MS(t)	(delay_ms(t))

#endif /* __W1_OPS_H__ */
