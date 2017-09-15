#ifndef RTC_UTILS_H
#define RTC_UTILS_H

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/exti.h>

#include <libopencm3/cm3/nvic.h>

/* RTC time in BCD format */
struct rtc_time {
	uint8_t pm;	/* 0: AM, 1: PM */
	uint8_t ht;	/* hour tens */
	uint8_t hu;	/* hour units */
	uint8_t mnt;	/* minute tens */
	uint8_t mnu;	/* minute units */
	uint8_t st;	/* second tens */
	uint8_t su;	/* second units */
};

/* RTC calendar date in BCD format */
struct rtc_date {
	uint8_t yt;	/* year tens */
	uint8_t yu;	/* year units */
	uint8_t wdu;	/* week day */
	uint8_t mt;	/* month tens */
	uint8_t mu;	/* month units */
	uint8_t dt;	/* date tens */
	uint8_t du;	/* date units */
};

/* RTC alarm date/time in BCD format */
struct rtc_alarm {

	uint8_t msk4;	/* date mask */
	uint8_t wdsel;	/* week day selection */
	uint8_t dt;	/* date tens */
	uint8_t du;	/* date units */

	uint8_t msk3;	/* hours mask */
	uint8_t pm;	/* 0: AM, 1: PM */
	uint8_t ht;	/* hour tens */
	uint8_t hu;	/* hour units */

	uint8_t msk2;	/* minutes mask */
	uint8_t mnt;	/* minute tens */
	uint8_t mnu;	/* minute units */

	uint8_t msk1;	/* seconds mask */
	uint8_t st;	/* second tens */
	uint8_t su;	/* second units */
};

void configure_next_alarm(int min, int sec);
void rtc_read_calendar(struct rtc_time *time, struct rtc_date *date);
void rtc_set_alarm(struct rtc_alarm *alarm);
void rtc_clear_alarm(void);
void rtc_setup(void);
void rtc_isr(void);

#endif /* RTC_UTILS_H */
