#include "USART.h"
#include "TIMER.h"
//#include "NEXTION.h"

#include "L298N.h"


#define IN1_PIN PD3
#define IN2_PIN PD4


int main(void) {

  USART_INIT(9600);
  L298N_INIT();
  MILLIS_INIT();
  sei(); //enable global interrupt
  program_start(); 

  //nextion_send_command("bkcmd=3");    //dont know dont ask
  //nextion_send_command("page page0"); //set correct page
  //nextion_send_command("page0.t0.txt=\"Status\"");

  while(1) {

    //nextion_handle_frame();
    //Forward, half speed
    motor_set_speed(1000);
    usart_send_string("1000");
    delay_ms(2000);

    // Coast (free spin)
    motor_coast();
    usart_send_string("Motor coast");
    delay_ms(1000);

    // Reverse, half speed
    motor_set_speed(-512);
    usart_send_string("-512");
    delay_ms(2000);

    // Brake (quick stop)
    motor_brake();
    usart_send_string("Quick break");
    delay_ms(1000);

    // Coast again
    motor_coast();
    usart_send_string("Motor coast");
    delay_ms(1000);


  }

  //shouldn't reach
  return 0;
}
