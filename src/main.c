#include "USART.h"
#include "TIMER.h"
#include "ADC.h"
#include "TB6612.h"
//#include "NEXTION.h"
#include "RPM.h"

//#include "L298N.h"


#define IN1_PIN PD3
#define IN2_PIN PD4


int main(void) {

  USART_INIT(9600);

  //L298N_INIT();
  TB6612_init();

  MILLIS_INIT();
  ADC_START();
  RPM_init();

  sei(); //enable global interrupt
  program_start(); 

  //nextion_send_command("bkcmd=3");    //dont know dont ask
  //nextion_send_command("page page0"); //set correct page
  //nextion_send_command("page0.t0.txt=\"Status\"");

  //Analog input 0
  DDRC &= ~(1<<PC3);
  PORTC &= ~(1 << PC3); //disables pull up resistor

  //button:
  DDRD &= ~(1 << PD7);
  PORTD |= (1 << PD7); //pull up resistor

  uint32_t lastPrint = millis();

  uint16_t speed = 0;
  uint16_t direction = 1;

  while(1) {

        
        // print ~10x per second
        if (millis() - lastPrint >= 100) {
          lastPrint = millis();
          float rpm = get_RPM();
          usart_send_string("RPM: "); usart_send_int((uint16_t)(rpm+0.5f)); usart_send_byte('\n');
        }

  }

  //shouldn't reach
  return 0;
}
