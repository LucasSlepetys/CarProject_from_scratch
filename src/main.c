#include "USART.h"
#include "TIMER.h"
#include "ADC.h"
#include "NEXTION.h"
#include "RPM.h"
#include "L298N.h"
#include "APP.h"
#include "MOTOR.h"

#include <avr/interrupt.h>
#include <avr/io.h>


int main(void) {
    
    
    USART_INIT(9600);
    L298N_INIT();
    MILLIS_INIT();
    ADC_START();
    RPM_init();
    app_init();
    motor_init();
    

    sei(); //enable global interrupt
    //program_start(); 

    nextion_send_command("bkcmd=3");    //dont know dont ask
    nextion_send_command("page page0"); //set correct page

    //Analog input 0
    DDRC &= ~(1<<PC3);
    PORTC &= ~(1 << PC3); //disables pull up resistor


    


    while(1) {

        nextion_handle_frame();
        app_update();

        
        
    }

    //shouldn't reach
    return 0;
}
