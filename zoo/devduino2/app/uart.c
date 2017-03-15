#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

#include "uart.h"

/* UART settings: 115200 8N1 */

void uart_init(void)
{
	UCSR0A = _BV(U2X0);
	UBRR0L = (F_CPU / (8UL * UART_BAUD)) - 1;

	/* enable tx */
	UCSR0B = _BV(TXEN0);
}

/* TX character: wait until tx holding register is empty */

int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);

	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;

	return 0;
}
