#include "PWM.h"

void PWM_INIT(void) {
    //timer 2

    //PORT D3 = OUTPUT of PWM
    DDRD |= (1 << PD3);

    //Clean start:
    TCCR2A = 0;                  
    TCCR2B = 0;

    //Wave form generation mode = fast pwm, 8 bit of resolution
    TCCR2A = (1 << WGM21) | (1 << WGM20);

    //NON-inverting mode, for fast pwm, for OCR2B, port D3
    TCCR2A |= (1 << COM2B1);

    // ===== Choose ONE prescaler =====
    // ~2 kHz: prescaler = 32  -> f = 16e6 / (32 * 256) ≈ 1953 Hz
    TCCR2B |= (1 << CS21) | (1 << CS20);     // CS22:0 = 0b011 => /32

    OCR2B  = 0; // start at 0% duty

}
