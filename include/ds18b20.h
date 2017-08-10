#ifndef __DS18B20_H__
#define __DS18B20_H__

/* Dallas/Maxim DS18B20 */

#define DS18B20_RES_ERR		1000
#define DS18B20_CRC_ERR		1001
#define DS18B20_CHK_ERR		1002

/* resolution */
typedef enum {
	R9BIT,
	R10BIT,
	R11BIT,
	R12BIT,
} res_t;

int ds18b20_set_res(res_t);
int ds18b20_read_temp(void);
int ds18b20_valid_temp(int);

#endif /* __DS18B20_H__ */
