#include <NEXTION.h>

#define FRAME_MAX 64

volatile uint8_t frame_buffer[FRAME_MAX]; //Should update every loop
volatile uint8_t frame_len = 0; //Should update every loop
volatile uint8_t frame_ready = 0; //Should update every loop

static uint8_t bytes_i = 0;     // how many bytes collected so far
static uint8_t ff_count = 0;  // how many 0xFF in a row

void collect_nextion_bytes(void) {

    //automatically volatile
    while(UCSR0A & (1 << RXC0)) { //while receiver is ready

        uint8_t one_byte_data = UDR0; //reads byte //reading clears RXC0

        if(bytes_i < FRAME_MAX) {
			frame_buffer[bytes_i++] = one_byte_data;
		} else {
			// overflow - restart and skip this command - hopefully doesnt happen
			bytes_i = 0;
			ff_count = 0;
			return;
		}

        //if 0xFF is encounted 3 times in a row: ff_count will be 3
        ff_count = (one_byte_data == 0xFF) ? (uint8_t)(ff_count + 1) : 0;

        //if end of a frame:
        if(ff_count == 3) {
			
			// frame completed: tell it to main loop
			frame_len = bytes_i;
			frame_ready = 1;
			
			bytes_i = 0;    //reset 
			ff_count = 0;	//reset
			
			return;
		}
    }

}

void print_completed_frame_line(void) {
	
	for (uint8_t i = 0; i < frame_len; i++) {
		print_hex_byte_as_ascii(frame_buffer[i]);
	}
	
	send_one_byte_data('\r');
	send_one_byte_data('\n');
	frame_ready = 0;  // clear flag
	
}

void nextion_send_command(const char *cmd) {
	
    send_string(cmd);
    send_one_byte_data(0xFF);
    send_one_byte_data(0xFF);
    send_one_byte_data(0xFF);
}
