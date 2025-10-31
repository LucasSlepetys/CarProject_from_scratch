//ENA -> D9 (OC1A, PWM)
//IN1 -> D3
//IN2 -> D4
//OUT1/OUT2 -> Motor

#include "L298N.h"
#include "PWM.h"

#define IN1_PIN PD3
#define IN2_PIN PD4

void L298N_INIT() {
    
    //INIT PWM for speed control
    PWM_INIT();

    //Make IN1_PIN and IN2_PIN as outputs
    DDRD |= (1 << IN1_PIN) | (1 << IN2_PIN);

    //starts with coast:
    motor_coast();

}

void motor_forward(void) {
    PORTD |= (1 << IN1_PIN);   // D3 HIGH
    PORTD &= ~(1 << IN2_PIN);   // D4 LOW
}

void motor_reverse(void) {
    PORTD &= ~(1 << IN1_PIN);   // D3 LOW
    PORTD |= (1 << IN2_PIN);   // D4 HIGH
}

void motor_brake(void) {
    PORTD |= (1 << IN1_PIN) | (1 << IN2_PIN); // both HIGH
}

void motor_coast(void) {
    PORTD &= ~((1 << IN1_PIN) | (1 << IN2_PIN)); // both LOW
}

void motor_set_speed(int16_t speed_1023) {

    if (speed_1023 > 1023) speed_1023 = 1023;
    if (speed_1023 < -1023) speed_1023 = -1023;

    if (speed_1023 >= 0) {
        motor_forward();
        OCR1A = (uint16_t)speed_1023; //PWM PORT //from 0 to 1023
    } else {
        motor_reverse();
        OCR1A = (uint16_t)(-speed_1023); //PWM PORT //from 0 to 1023 //Makes speed positive again
    }
}