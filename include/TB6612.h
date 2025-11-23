// #ifndef TB6612_H_
// #define TB6612_H_

// #include "system.h"

// /*
//  * TB6612FNG Motor Driver (Single-Motor, Channel A)
//  *
//  * Default wiring (override with -D or #define before including this header):
//  *   VM   -> Motor supply (e.g., 6–12 V)
//  *   VCC  -> 5 V (logic)
//  *   GND  -> common ground (Arduino, battery, driver)
//  *
//  *   PWMA -> D9  (OC1A, PWM)     [handled by your PWM_init()]
//  *   AIN1 -> D3  (PD3)
//  *   AIN2 -> D4  (PD4)
//  *   STBY -> D2  (PD2)  <-- MUST be driven HIGH to enable the driver
//  *
//  *   A01/A02 -> Motor terminals
//  *
//  * Notes:
//  *  - Use Timer1 Fast PWM (10-bit) with prescaler=1 (~15.6 kHz) for quiet motors.
//  *  - Call PWM_init() before/inside TB6612_init().
//  *  - Speed range: -1023..+1023 (sign = direction).
//  */

// // Initialize TB6612 (sets direction + STBY pins, calls PWM_init())
// void TB6612_init(void);

// // Basic motion controls
// void motor_forward(void);  // AIN1=1, AIN2=0
// void motor_reverse(void);  // AIN1=0, AIN2=1
// void motor_brake(void);    // AIN1=1, AIN2=1 (short brake)
// void motor_coast(void);    // AIN1=0, AIN2=0 (freewheel)

// // Speed control: -1023..+1023 (10-bit)
// //   >=0 -> forward, <0 -> reverse
// void motor_set_speed(int16_t speed_1023);

// #endif /* TB6612_H_ */
