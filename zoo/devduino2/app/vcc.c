#include <util/delay.h>
#include <avr/io.h>

#include "adc.h"

long read_vcc(void)
{
	long result;

	/* select Vref = internal 1v1 , select Vin = A2(ADC2) */

	adc_scm_init(0x3, 0x2);

	/* XXX Limitation of this approach
	 * Here we rely upon the fact that Vcc is less than 3.3v on devDuino2.
	 * In this case Vin = Vcc * 470 / 1470 <= 1.06v < Vref = 1.1v.
	 * Hence Vin/Vref < 1 and convertion result ADC = (Vin/Vref)*1024
	 * fits to 10bit resolution.
	 */

	result = adc_scm_read();

	/*
	 * calculate Vcc in mV
	 * Vcc = Vx * 1470 / 470 where Vx is a voltage on R2
	 * ADC = Vin*1024/Vref where Vin = Vx and Vref = 1v1
	 * ADC = Vx*1024/1100 =>
	 * Vcc = ADC * 1100 * 1470 / 470 / 1024
	 */

	result = result * 40425 / 12032;

	return result;
}

long read_vcc_alt(void)
{
	long result;

	/* select Vref = AVcc = Vcc, select Vin = internal 1v1 */

	adc_scm_init(0x1, 0xe);

	/* XXX Feature of this approach
	 * In this approach Vref is Vcc. For some reason for Vref to settle.
	 * Several first conversions provide incorrect result (less than
	 * the correct value) if no delay_ms(50) between ADC enablement
	 * and conversion start.
	 */

	result = adc_scm_read();

	/*
	 * calculate Vcc in mV
	 * ADC = Vin*1024/Vref =>
	 * ADC = 1.1*1000*1024/Vcc =>
	 * Vcc = 1126400/ADC
	 */
	result = 1126400L / result;

	return result;
}
