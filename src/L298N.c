//ENA -> D3 (OC2B, PWM)
//IN1 -> D4
//IN2 -> D5
//OUT1/OUT2 -> Motor

#include "L298N.h"
#include "PWM.h"

#define IN1_PIN PD4
#define IN2_PIN PD5

void L298N_INIT() {
    
    //INIT PWM for speed control
    PWM_INIT();

    //Make IN1_PIN and IN2_PIN as outputs
    DDRD |= (1 << IN1_PIN) | (1 << IN2_PIN);

    //starts with coast:
    motor_coast();

}

void motor_forward(void) {
    PORTD |= (1 << IN1_PIN);   
    PORTD &= ~(1 << IN2_PIN);   
}

void motor_reverse(void) {
    PORTD &= ~(1 << IN1_PIN);   
    PORTD |= (1 << IN2_PIN);   
}

void motor_brake(void) {
    PORTD |= (1 << IN1_PIN) | (1 << IN2_PIN); // both HIGH
}

void motor_coast(void) {
    PORTD &= ~((1 << IN1_PIN) | (1 << IN2_PIN)); // both LOW
}

void motor_set_speed(int16_t speed_1023) {

    // -1023 .. 1023
    if (speed_1023 > 1023)  speed_1023 = 1023;
    if (speed_1023 < -1023) speed_1023 = -1023;

    // If zero then coast (or brake) and 0% duty
    if (speed_1023 == 0) {
        motor_coast();
        OCR2B = 0;
        return;
    }

    // Magnitude: 0..1023
    uint16_t mag = (speed_1023 >= 0) ? speed_1023 : -speed_1023;

    // Scale 0..1023 to 0..255
    uint8_t duty = (uint8_t)((uint32_t)mag * 255 / 1023);

    // Set direction based on sign
    if (speed_1023 > 0) {
        motor_forward();
    } else {    // speed_1023 < 0
        motor_reverse();
    }

    // Apply duty
    OCR2B = duty;
}
