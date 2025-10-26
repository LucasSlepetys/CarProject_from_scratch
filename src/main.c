#include "USART.h"
#include "NEXTION.h"

int main(void) {

  init_usart();

  while(1) {

      collect_nextion_bytes(); //non-blocking read
      if(frame_ready) print_completed_frame_line(); //print full frame //clear frame_ready



  }
  //shouldn't reach
  return 0;
}
