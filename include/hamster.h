#ifndef HAMSTER_H_H
#define HAMSTER_H_H

#include <RF24.h>

/* radio settings */

#define NRF_ADDR	{ 0xE1, 0xE1, 0xE1, 0xE1, 0xE1 }
#define NRF_CHAN	10
#define NRF_RATE	RF24_RATE_250K
#define NRF_CRC		RF24_CRC_16_BITS
#define NRF_PA		RF24_PA_MIN

/* sensor IDs */

#define SID_TEMP_C	10
#define SID_RANGE_MM	20
#define SID_VOLT_MV	30

#endif	/* HAMSTER_H_H */
