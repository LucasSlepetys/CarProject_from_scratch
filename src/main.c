#include "USART.h"
#include "NEXTION.h"


int main(void) {

  USART_INIT(9600);
  sei(); //enable global interrupt
  program_start(); 

  nextion_send_command("bkcmd=3");
  nextion_send_command("page page0"); // replace page0 with your actual page name


  

  nextion_send_command("page0.t0.txt=\"Status\"");
  while(1) {

    nextion_handle_frame();
    


  }
  //shouldn't reach
  return 0;
}
