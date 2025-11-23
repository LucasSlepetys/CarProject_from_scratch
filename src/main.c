#include "USART.h"
#include "TIMER.h"
#include "ADC.h"
#include "NEXTION.h"
#include "RPM.h"
#include "L298N.h"

#include <avr/interrupt.h>
#include <avr/io.h>

// These are not needed if L298N.c defines its own IN1/IN2 pins
// #define IN1_PIN PD3
// #define IN2_PIN PD4

// -------- state machine for motor test --------
typedef enum {
    MOTOR_FORWARD,
    MOTOR_STOP,
    MOTOR_REVERSE
} MotorState;

int main(void) {

    USART_INIT(9600);

    L298N_INIT();
    //TB6612_init();

    MILLIS_INIT();
    ADC_START();
    RPM_init();
    

    sei(); //enable global interrupt
    //program_start(); 

    nextion_send_command("bkcmd=3");    //dont know dont ask
    nextion_send_command("page page0"); //set correct page

    //Analog input 0
    DDRC &= ~(1<<PC3);
    PORTC &= ~(1 << PC3); //disables pull up resistor

    //button:
    DDRD &= ~(1 << PD7);
    PORTD |= (1 << PD7); //pull up resistor

    // -------- NEW: timing variables --------
    //uint32_t lastPrint       = millis();  // for RPM printing (100 ms)
    //uint32_t lastMotorChange = millis();  // for motor state changes (1000 ms)

    //MotorState motorState = MOTOR_FORWARD;   // start going forward
    while(1) {

        uint32_t now = millis();
        nextion_handle_frame();
        app_update();
       

        // ---------- change motor state every 1000 ms ----------
        // if (now - lastMotorChange >= 1000) {  // 1 second
        //     lastMotorChange = now;

        //     switch (motorState) {
        //         case MOTOR_FORWARD:
        //             motor_set_speed(1000);    // forward, high speed
        //             motorState = MOTOR_STOP;
        //             break;

        //         case MOTOR_STOP:
        //             motor_set_speed(0);       // stop
        //             motorState = MOTOR_REVERSE;
        //             break;

        //         case MOTOR_REVERSE:
        //             motor_set_speed(-1000);   // reverse
        //             motorState = MOTOR_FORWARD;
        //             break;
        //     }
        // }

        // no delay_ms() anywhere → fully non-blocking
    }

    //shouldn't reach
    return 0;
}
