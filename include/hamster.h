#ifndef HAMSTER_H_H
#define HAMSTER_H_H

#include <RF24.h>

/* radio settings */

#define NRF_ADDR	{ 0xE1, 0xE1, 0xE1, 0xE1, 0xE1 }
#define NRF_CHAN	110
#define NRF_RATE	RF24_RATE_250K
#define NRF_CRC		RF24_CRC_16_BITS
#define NRF_PA		RF24_PA_MAX
#define NRF_LNA_PA	RF24_PA_MIN

/* sensor IDs */

#define SID_TEMP_C(i)	(0x10 + (i))
#define SID_RANGE_SM(i)	(0x20 + (i))
#define SID_VOLT_MV(i)	(0x30 + (i))

#define SID_MASK	0xF

/* alert IDs */

#define AID_NODE_ERR	0x1000
#define AID_WATER_LVL	0x1001
#define AID_VBAT_LOW	0x1002

#endif	/* HAMSTER_H_H */
