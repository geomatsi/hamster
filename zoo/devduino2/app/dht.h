/*
 * Copyright (C) 2016 Jacek Wieczorek
 * Copyright (C) 2018 geomatsi (geomatsi@gmail.com)
 *
 * This software may be modified and distributed
 * under the terms of the MIT license.
 *
 * Based on:
 * - avr-dhtxx at https://github.com/Jacajack/avr-dhtxx
 *
 */

#ifndef _AVR_DHT_H_
#define _AVR_DHT_H_

/* DHT11 and DHT22 temperature/humidity sensors */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
	DHT_TYPE_DHT11,
	DHT_TYPE_DHT22,
} dht_type_t;

typedef enum {
	DHT_OK,
	DHT_ERR_SUPP,
	DHT_ERR_COMM,
	DHT_ERR_CSUM,
} dht_res_t;

void dht_power_up();
void dht_power_down();

dht_res_t dht_read(dht_type_t type, uint8_t *t_int, uint8_t *t_dec, uint8_t *h_int, uint8_t *h_dec);

#endif /* _AVR_DHT_H_ */
