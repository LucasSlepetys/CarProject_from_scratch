// #ifndef L298N_H_
// #define L298N_H_

// #include "system.h"

// /*
//  * L298N motor driver library
//  *
//  * Wiring:
//  *   ENA -> D9  (OC1A, PWM)
//  *   IN1 -> D3  (PD3)
//  *   IN2 -> D4  (PD4)
//  *   OUT1/OUT2 -> Motor
//  *
//  * Description:
//  *   - Uses Timer1 (OC1A) for PWM speed control
//  *   - IN1/IN2 control motor direction
//  *   - Supports forward, reverse, brake, and coast
//  *   - Speed input range: -1023 .. +1023
//  */

// // Initialize L298N motor driver and PWM
// void L298N_INIT(void);

// // Motor control functions
// void motor_forward(void);
// void motor_reverse(void);
// void motor_brake(void);
// void motor_coast(void);

// // Set motor speed (-1023 to +1023)
// // Positive -> forward, Negative -> reverse
// void motor_set_speed(int16_t speed_1023);

// #endif /* L298N_H_ */
