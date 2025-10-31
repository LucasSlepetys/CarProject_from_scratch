#include "PWM.h"

void PWM_INIT(void) {

    //PORT D9 = OUTPUT of PWM
    DDRB |= (1 << PB1);

    //Clean start:
    TCCR1A = 0;                  
    TCCR1B = 0;

    //Wave form generation mode = fast pwm, 10 bit of resolution
    TCCR1B |= (1 << WGM12);
    TCCR1A |= (1 << WGM10) | (1 << WGM11); 

    //NON-inverting mode, for fast pwm, for OCR1A, port D9
    TCCR1A |= (1 << COM1A1);

    //Prescaler for motor control: 1
    //TCCR1B |= (1 << CS10); //f_pwm = 16kHz

    //Prescaler for LED bridgtess or L298N: 8
    TCCR1B |= (1 << CS11); //f_pwm = 2kHz

    OCR1A = 0; // start at 0% duty

}

// void LED_brightness(uint16_t brightness) {

//     if (brightness > 1023) brightness = 1023;
//     OCR1A = brightness; //LED PORT //from 0 to 1023
// }

