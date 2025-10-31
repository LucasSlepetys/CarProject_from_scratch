#include "ADC.h"

void ADC_START(void){ 

    //Reference voltage:
    ADMUX = (1 << REFS0); // REFS0 = AVcc reference //ADC channel 0 by default
    
    //ADC enable //ADC Prescaler
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable + 128 division factor, 125Khz

    // Dummy conversion after selecting reference
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    (void)ADC;
}

uint16_t adc_read(uint8_t channel) {

    //specifing channel
    ADMUX = (ADMUX & 0b11110000) | (channel & 0b00001111);

    // ---- dummy conversion to flush S/H after MUX change ----
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    (void)ADC;

    ADCSRA |= (1 << ADSC); //Start conversion 

    //while it is still converting stay here
    //! Could add interrupt instead
    while(ADCSRA & (1 << ADSC));
    return ADC;

}