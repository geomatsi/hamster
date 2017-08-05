#ifndef HAMSTER_H_H
#define HAMSTER_H_H

#include <RF24.h>

#define NRF_ADDR	{ 0xE1, 0xE1, 0xE1, 0xE1, 0xE1 }
#define NRF_CHAN	10
#define NRF_RATE	RF24_RATE_250K
#define NRF_CRC		RF24_CRC_16_BITS
#define NRF_PA		RF24_PA_MIN

#endif	/* HAMSTER_H_H */
