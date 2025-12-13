#include "motor.h"
#include "RPM.h"         
#include "L298N.h"
#include "TIMER.h"

//! --- Wheel geometry ---
#define WHEEL_DIAMETER_M   0.045f
#define PI_F               3.14159265f
#define WHEEL_CIRC_M       (PI_F * WHEEL_DIAMETER_M)  // circumference = pi*D

//! --- Constants for PI control ---
// Units: error is m/s // !To be tuned 
#define KP  1800.0f
#define KI  600.0f     

// --- Output limits, Units: output is PWM --
#define PWM_MAX 1023
#define PWM_MIN_MOVE 300    //! to be tune (stall threshold)

// --- Time control ---
#define CTRL_PERIOD_MS 20u //20 mili seconds
#define DT_S (CTRL_PERIOD_MS / 1000.0f)  // 0.02 seconds

// Anti-windup clamp on integral term (PWM units)
#define ITERM_MIN (-400.0f)
#define ITERM_MAX (400.0f)

        
static float i_term = 0.0f;
static int16_t last_u = 0;
static uint32_t last_time_ms = 0;

static float last_measured_speed = 0.0f;


float get_actual_speed() {

    float rpm = get_RPM();
    return (WHEEL_CIRC_M * rpm) / 60.0f;

}

void motor_init(void)
{
    i_term = 0.0f;
    last_u = 0;
    
    last_time_ms = millis();
}

void motor_reset_controller(void) {
    i_term = 0.0f;
    last_u = 0;
} 

float update_motor(float target_speed) {

    uint32_t now = millis();
    
    if((uint32_t) (now - last_time_ms) < CTRL_PERIOD_MS) return last_measured_speed;
    last_time_ms += CTRL_PERIOD_MS;

    if(target_speed < 0) return last_measured_speed;

    //speed in m/s
    last_measured_speed = get_actual_speed();

    float error = target_speed - last_measured_speed;
    

    i_term +=  KI * error * DT_S; //integral term update (PWM units)

    if(i_term > ITERM_MAX) i_term = ITERM_MAX;
    if(i_term < ITERM_MIN) i_term = ITERM_MIN;

    float u_f = (KP * error) + i_term; //KP in RPM units / (m/s) //u units is RPM units

    if (u_f > PWM_MAX) u_f = PWM_MAX;
    if (u_f < 0.0f) u_f = 0.0f; //forward speed only


    int16_t u = (int16_t)u_f;

    // add minimum PWM in order for motor not to stall (needs tunning)
    if (target_speed > 0.01f && u > 0 && u < PWM_MIN_MOVE) u = PWM_MIN_MOVE;


    // if target is ~0, stop and reset integrator
    if (target_speed <= 0.01f) {
        i_term = 0.0f;
        u = 0;
    }

    last_u = u;

    motor_set_speed(u);

    return last_measured_speed;
}

