
// #include "TB6612.h"
// #include "PWM.h"

// // Pin map (Arduino Uno/Nano)
// #define AIN1   PD3   // D3
// #define AIN2   PD4   // D4
// #define STBY   PD2   // D2
// // PWMA is on OC1A (D9 = PB1) via Timer1 from your existing PWM_init()

// void TB6612_init(void) {
//     // PWM for speed (OC1A/D9)
//     PWM_INIT();                      // prescaler = 1 for ~15.6 kHz

//     // Direction + standby pins as outputs
//     DDRD |= (1 << AIN1) | (1 << AIN2) | (1 << STBY);

//     // Leave motor coasting, and exit standby
//     motor_coast();
//     PORTD |= (1 << STBY);            // STBY HIGH -> enabled
// }

// void motor_forward(void) {
//     PORTD |=  (1 << AIN1);
//     PORTD &= ~(1 << AIN2);
// }

// void motor_reverse(void) {
//     PORTD &= ~(1 << AIN1);
//     PORTD |=  (1 << AIN2);
// }

// void motor_brake(void) {
//     PORTD |= (1 << AIN1) | (1 << AIN2);  // short brake
// }

// void motor_coast(void) {
//     PORTD &= ~((1 << AIN1) | (1 << AIN2)); // coast
// }

// void motor_set_speed(int16_t speed_1023) {
//     if (speed_1023 >  1023) speed_1023 =  1023;
//     if (speed_1023 < -1023) speed_1023 = -1023;

//     if (speed_1023 >= 0) {
//         motor_forward();
//         OCR2B  = (uint16_t)speed_1023;        // 0..1023
//     } else {
//         motor_reverse();
//         OCR2B  = (uint16_t)(-speed_1023);
//     }
// }
