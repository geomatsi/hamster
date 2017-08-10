#include <inttypes.h>

#include "ds18b20.h"
#include "w1.h"

/* commands */

#define SEARCH_ROM	0xF0
#define READ_ROM	0x33
#define MATCH_ROM	0x55
#define SKIP_ROM	0xCC
#define CONVERT_T	0x44
#define WRITE_PAD	0x4E
#define READ_PAD	0xBE
#define COPY_PAD	0x48

/* */

static res_t resolution = R12BIT;

static int ds18b20_conv_temp(uint8_t msb, uint8_t lsb)
{
	uint16_t val;
	int temp;

	/* TODO: currently support only default 12bit resolution */

	switch (resolution) {
		case R12BIT:
			val = (msb << 8) | lsb;
			temp = (val & 0x07FF) >> 4;
			temp = (val & 0xF800) ? -temp : temp;
			break;
		case R9BIT:
		case R10BIT:
		case R11BIT:
		default:
			temp = DS18B20_RES_ERR;
			break;
	}

	return temp;
}


static int ds18b20_crc_check(const uint8_t *data, uint32_t len)
{
	uint8_t crc, byte;
	uint8_t fst, inb;
	int i;

	crc = 0x0;

	while (len--) {
		byte = *data++;

		for (i = 0; i < 8; i++) {
			inb = (byte >> i) & 0x1;
			fst = crc & 0x1;
			crc = (fst ^ inb) ? ((crc >> 1) ^ 0x8c) : (crc >> 1);
		}
	}

	return (crc == 0x0);
}

/* */

int ds18b20_set_res(res_t res)
{
	int ret = 1;

	/* TODO: currently support only default 12bit resolution */

	switch (res) {
		case R9BIT:
			ret = 0;
			break;
		case R10BIT:
			ret = 0;
			break;
		case R11BIT:
			ret = 0;
			break;
		case R12BIT:
			resolution = res;
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}

int ds18b20_read_temp(void)
{
	uint8_t data[9];
	int32_t temp;
	uint8_t i;

	/* reset and check presence */
	if (!w1_init_transaction())
		return DS18B20_CHK_ERR;

	/* skip ROM: next command can be broadcasted */
	w1_send_byte(SKIP_ROM);

	/* start single temperature conversion */
	w1_send_byte(CONVERT_T);

	/* temperature conversion takes ~1sec */
	w1_delay_ms(1000);

	/* reset and check presence */
	if (!w1_init_transaction())
		return DS18B20_CHK_ERR;

	/* skip ROM: careful !!! works only for one device on bus: next command is unicast */
	w1_send_byte(SKIP_ROM);

	/* read scratchpad */
	w1_send_byte(READ_PAD);

	/* get all scratchpad bytes */
	for (i = 0; i < 9; i++)
		data[i] = w1_recv_byte();

	/* check crc */
	if (!ds18b20_crc_check(data, 9))
		return DS18B20_CRC_ERR;

	/* calculate temperature */
	temp = ds18b20_conv_temp(data[1], data[0]);

	return temp;
}

int ds18b20_valid_temp(int temp)
{
	return ((-200 <= temp) && temp <= 200) ? 1 : 0;
}
