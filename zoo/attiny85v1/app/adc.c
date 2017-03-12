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
	 *   -- current limitation: REFS2 is always zero
	 *   -- ref = 0 = 00b => Vcc used as voltage reference
	 *   -- ref = 1 = 01b => External voltage reference at PB0 pin
	 *   -- ref = 2 = 10b => Internal 1v1 voltage reference
	 *   -- ref = 3 = 11b => not used
	 *
	 * - Pinmux: analog channel
	 *   -- mux = 0 = 0000b => ADC0 (PB5)
	 *   -- mux = 1 = 0001b => ADC1 (PB2)
	 *   -- mux = 2 = 0010b => ADC2 (PB4)
	 *   -- mux = 3 = 0011b => ADC3 (PB3)
	 *   -- mux = 12 = 1100b => internal Vbg as input voltage
	 *
	 * - Disable ADC Left Adjust Result: ADLAR = 0
	 */

	ADMUX = ((vref & 0x3) << REFS0) | (mux & 0xf);

	/* Simple ADCSRA setup: no complete spec coverage
	 *
	 * - Enable ADC: ADEN = 1
	 * - Setup single conversion mode (disable auto trigger): ADATE = 0
	 * - Disable interrupts: ADIF = ADIE = 0
	 * - Set prescaler to 8 : ADPS[2:0] = 011b
	 *   -- ADC clock: 1MHz / 8 = 125kHz
	 *   -- TODO calculation needed based on F_CPU:
	 *      --- F_CPU = 1 MHz => 8 (125kHz) or 16 (62.5kHz)
	 *      --- F_CPU = 4 MHz => 32 (125kHz) or 64 (62.5kHz)
	 *      --- F_CPU = 8 MHz => 64 (125kHz) or 128 (62.5kHz)
	 *      --- F_CPU = 16 MHz => 128 (125kHz)
	 */

	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
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
