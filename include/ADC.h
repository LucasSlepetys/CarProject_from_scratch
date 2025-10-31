#ifndef ADC_H
#define ADC_H

#include <system.h>

void ADC_START(void);
uint16_t adc_read(uint8_t channel);


#endif /* ADC_H */