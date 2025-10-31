#include "TIMER.h"

static volatile uint32_t ms_counter = 0;

void MILLIS_INIT(void) {

    // CTC mode (clear timer on compare)
    TCCR0A |= (1 << WGM01);

    //64 prescaller with 16MHz cpu clock, f_timer = 250kHz
    //So one timer tick = 4 microseconds

    //Timer resets whenver it reaches OCR1A value = 250 counts since 4 microsenconds x X = 1 ms
    //Each increase in count takes 4 microseconds 
    //Therefore timer resets after 1 ms

    OCR0A = 249;
    TCCR0B |= (1 << CS01) | (1 << CS00); //prescaler = 64

    // Enable compare match interrupt
    TIMSK0 |= (1 << OCIE0A);
}

//Interrupt called when count is reset, after 250 counts after 1 ms
ISR(TIMER0_COMPA_vect) {

    ms_counter++;

}


uint32_t millis(void) {

    uint8_t sreg = SREG; //save interrupt state
    cli(); // disable interrupts temporatily
    uint32_t ms = ms_counter; //4 bytes, 1 call for each byte
    SREG = sreg; //restore interrupts

    return ms;
}

// Blocking delay built on millis()
void delay_ms(uint32_t ms) {
    uint32_t start = millis();

    while ((millis() - start) < ms) {
        //wait until time passes
    }
}