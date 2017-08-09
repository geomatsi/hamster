#ifndef __HC_SR04_H__
#define __HC_SR04_H__

/* Ultrasonic range sensor HC_SR04 */

void hc_sr04_init(uint32_t freq);
void hc_sr04_setup_echo_capture(void);
void hc_sr04_trigger_pulse(void);
int hc_sr04_valid_range(uint32_t range);
uint32_t hc_sr04_get_range(void);

#endif /* __HC_SR04_H__ */
