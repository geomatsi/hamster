#include <util/delay.h>
#include <avr/io.h>

#include "adc.h"

long read_temp_mcp9700(void)
{
	long result;

	/* select Vref = internal 1v1, select Vin = A3(ADC3) */

	adc_scm_init(0x3, 0x3);

	/* XXX Limitation of this approach
	 * Here we rely upon the fact that Vtemp is less than 1v1.
	 * According to MCP9700 specs Vtemp is in [0v0, 1v1] for reasonable
	 * range of temperatures: from -50C to 50C.
	 */

	result = adc_scm_read();

	/*
	 * calculate Temp from ADC value
	 * AVR spec: ADC = Vin*1024/Vref where Vref = 1v1 and Vin = Vtemp
	 * MCP9700 spec: Vtemp = Tc*T + V0 where Tc = 10 mV/C and V0 = 500mV
	 * Finally T = 110*ADC/1024 - 50
	 */

	result = result * 110 / 1024 - 50;

	return result;
}
