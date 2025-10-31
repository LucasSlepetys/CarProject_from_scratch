#include <NEXTION.h>

#define FRAME_MAX 64

volatile uint8_t frame_buffer[FRAME_MAX]; //Should update every loop
volatile uint8_t frame_len = 0; //Should update every loop
volatile uint8_t frame_ready = 0; //Should update every loop


//Interrupt for receiver:   
//whenver enabled: ready to receive
ISR(USART_RX_vect) {

	static uint8_t bytes_i = 0;     // how many bytes collected so far
	static uint8_t ff_count = 0;  // how many 0xFF in a row

	uint8_t one_byte_data = UDR0; //reads byte //reading clears recevier interrupt

	if (frame_ready == 1) return;

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

void handle_frame(const uint8_t *buffer, uint8_t len ) {

	uint8_t header = buffer[0];

	switch(header) {

		//! Won't work if USART is being used to send debugging messages or other things


		// case 0x00: usart_send_string((const uint8_t *)"EE 00: Invalid Instruction"); break;
		// case 0x1A: usart_send_string((const uint8_t*)"ERR 1A: invalid name\r\n"); break;
		// case 0x23: usart_send_string((const uint8_t*)"ERR 23: invalid attribute\r\n"); break;

		case 0x65: //Touch event: 65 page id event

			if(len == 7) {

				uint8_t page = buffer[1];
				uint8_t id = buffer[2];
				uint8_t event = buffer[3]; // 1=press, 0=release

				// trigger on release (event==0). Use object name only.
				if (page==0 && id==2 && event==0) nextion_send_command("t0.txt=\"ON\"");
            	if (page==0 && id==3 && event==0) nextion_send_command("t0.txt=\"OFF\"");

			}
			break;

		default:
        // Unknown/less common header—log if needed
        break;
    }
}


void nextion_handle_frame(void) {

	if (frame_ready == 0) return; //frame is not finished

	uint8_t local_frame_len;
	uint8_t local_frame_buffer[FRAME_MAX];

	uint8_t sreg = SREG; cli(); //save current interrupt state //disable interrupt for a short time
	local_frame_len = frame_len;
	for (uint8_t i = 0; i < local_frame_len; i++) local_frame_buffer[i] = frame_buffer[i];
    frame_ready = 0;   // release buffer for ISR
	SREG = sreg; //enable interrupt from last state

	//print received command, only if not a status 1A from nextion:
	if (!(local_frame_len == 4 && local_frame_buffer[0] == 0x01 &&
		local_frame_buffer[1] == 0xFF && local_frame_buffer[2] == 0xFF && local_frame_buffer[3] == 0xFF)) {

			//for (uint8_t i = 0; i < local_frame_len; i++) {
			//	print_hex_byte_as_ascii(local_frame_buffer[i]);
			//} //can be commented out
			//usart_send_byte('\r');
			//usart_send_byte('\n');

			handle_frame(local_frame_buffer, local_frame_len); //handles inputs
		
		}

	
}


void nextion_print_FFs() {
	for (int i = 0; i < 3; i++) usart_send_byte(0xFF);
}

void nextion_send_command(const char *cmd) {
    usart_send_string((const uint8_t *)cmd);
    nextion_print_FFs();
}
