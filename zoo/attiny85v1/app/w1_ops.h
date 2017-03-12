#ifdef _AVR_W1_OPS_H_
#error "w1 ops has been already defined"
#else
#define _AVR_W1_OPS_H_

#include <avr/io.h>

#include <stdbool.h>

/* platform- and app- specific 1-wire pin control */

#define SET_PIN_HIGH()		\
	do {					\
		DDRB |= _BV(DDB3);	\
		PORTB |= _BV(PB3);	\
	} while (0)

#define SET_PIN_LOW()		\
	do {					\
		DDRB |= _BV(DDB3);	\
		PORTB &= ~_BV(PB3);	\
	} while (0)

#define GET_PIN_VALUE()	(PINB & _BV(PB3))

#endif /* _AVR_W1_OPS_H_ */
