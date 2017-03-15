#include <util/delay.h>
#include <avr/io.h>

#include "adc.h"

/* ADC: single conversion mode */

void adc_enable(void)
{
	ADCSRA |= _BV(ADEN);
}

void adc_disable(void)
{
	ADCSRA &= ~_BV(ADEN);
}

void adc_scm_init(uint8_t vref, uint8_t mux)
{
	/* Simple ADMUX setup: no complete spec coverage
	 *
	 * - Voltage reference:
	 *   -- ref = 0 = 00b => AREF, internal Vref turned off
	 *   -- ref = 1 = 01b => AVcc with ext capacitor at AREF
	 *   -- ref = 2 = 10b => reserved
	 *   -- ref = 3 = 11b => Internal Vref = 1v1 with ext capacitor at AREF
	 *
	 * - Pinmux: input analog channel
	 *   -- mux = 0 = 0000b => ADC0
	 *   -- mux = 1 = 0001b => ADC1
	 *   -- mux = 2 = 0010b => ADC2
	 *   -- mux = 3 = 0011b => ADC3
	 *   -- mux = 4 = 0100b => ADC4
	 *   -- mux = 5 = 0101b => ADC5
	 *   -- mux = 6 = 0110b => ADC6
	 *   -- mux = 7 = 0111b => ADC7
	 *
	 * - Disable ADC Left Adjust Result: ADLAR = 0
	 */

	ADMUX = ((vref & 0x3) << REFS0) | (mux & 0xf);

	/* Simple ADCSRA setup: no complete spec coverage
	 *
	 * - Enable ADC: ADEN = 1
	 * - Setup single conversion mode (disable auto trigger): ADATE = 0
	 * - Disable interrupts: ADIF = ADIE = 0
	 * - Set prescaler to 128 : ADPS[2:0] = 011b
	 *   -- ADC clock: 16MHz / 128 = 125kHz
	 */

	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

uint16_t adc_scm_read(void)
{
	uint16_t r, h, l;

	/* start conversion */
	ADCSRA |= _BV(ADSC);

	/* wait until conversion is completed */
	while (bit_is_set(ADCSRA, ADSC));

	/* read result
	 * NB: ADCL must be read first
	 * When ADCL is read then ADC[LH] is not updated until ADCH is read
	 */
	l = ADCL;
	h = ADCH;
	r = (h << 8) | l;

	return r;
}
