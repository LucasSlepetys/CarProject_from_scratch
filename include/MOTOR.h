#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
float update_motor(float target_speed);

#endif
