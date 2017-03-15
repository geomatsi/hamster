#ifndef _AVR_LEDS_H_
#define _AVR_LEDS_H_

void leds_init(void);
void led_on(uint32_t led);
void led_off(uint32_t led);
void led_toggle(uint32_t led);

#endif /* _AVR_LEDS_H_ */
