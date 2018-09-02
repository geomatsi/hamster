#ifdef _AVR_DHT_OPS_H_
#error "DHT ops has been already defined"
#else
#define _AVR_DHT_OPS_H_

#include <avr/io.h>

/* platform- and app- specific dht data/pwr pins control */

#define DHT_PWR_ENABLE()		\
	do {				\
		DDRD |= _BV(DDD5);	\
		PORTD |= _BV(PD5);	\
	} while (0)

#define DHT_PWR_DISABLE()		\
	do {				\
		DDRD |= _BV(DDD5);	\
		PORTD &= ~_BV(PD5);	\
	} while (0)

#define DHT_SET_PIN_HIGH()		\
	do {				\
		DDRD |= _BV(DDD3);	\
		PORTD |= _BV(PD3);	\
	} while (0)

#define DHT_SET_PIN_LOW()		\
	do {				\
		DDRD |= _BV(DDD3);	\
		PORTD &= ~_BV(PD3);	\
	} while (0)

#define DHT_PIN_INPUT()			\
	do {				\
		DDRD &= ~_BV(DDD3);	\
		/* pull-up */		\
		PORTD |= _BV(PD3);	\
	} while (0)

#define DHT_GET_PIN_VALUE()	(PIND & _BV(PD3))

#endif /* _AVR_DHT_OPS_H_ */
