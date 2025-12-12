#include "motor.h"
#include "RPM.h"         
#include "L298N.h"

//! --- Wheel geometry ---
#define WHEEL_DIAMETER_M   0.045f
#define PI_F               3.14159265f
#define WHEEL_CIRC_M       (PI_F * WHEEL_DIAMETER_M)  // circumference = pi*D



void motor_init(void)
{

    L298N_INIT();
}

