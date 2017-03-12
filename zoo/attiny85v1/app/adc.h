#ifndef _AVR_ADC_H_
#define _AVR_ADC_H_

/* ADC single conversion mode */

void adc_enable(void);
void adc_disable(void);
void adc_scm_init(uint8_t vref, uint8_t mux);
uint16_t adc_scm_read(void);

#endif /* _AVR_ADC_H_ */
