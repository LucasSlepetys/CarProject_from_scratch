#include "NEXTION.h"
#include "APP.h"

//! ----------------------------------------------------------------------------
//Handles frame
volatile uint8_t frame_buffer[FRAME_MAX]; //Should update every loop
volatile uint8_t frame_len = 0; //Should update every loop
volatile uint8_t frame_ready = 0; //Should update every loop
//! ----------------------------------------------------------------------------



//! ----------------------------------------------------------------------------
//Interrupt gets called whenver USART receives new byte of data
//Every byte of data is stored in frame_buffer array
/* Whenver frame_buffer overflows, it restarts:
	! This would only happen if the command received is enourmous or if another device is using USART 
	! together with Nextion display
*/ 
//A frame is said to be ready when it received 3 0xFF in a row (Standard way nextion sends a command)
//When a frame is ready the flag frame_ready is updated to 1
//Immedially after the buffer is resetted, therefore immediate action should take place whenver frame is ready
//! If no action is taken palce and flag is not reseted to 0, then ISR will not do anything and data will be lost
//! ----------------------------------------------------------------------------
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

//! ----------------------------------------------------------------------------
//Function is continuously called in main loop
//Whenever frame_ready flag is 1, function will run completely
//In order to make it non-blocking, it copies the frame_buffer into a local_frame_buffer:
	//! It stops all interrupts to make the copy in order not to lose data
	//! then it resets all interrupts and sets frame_ready back to 0 to allow ISR to happen
//If status is not equal to 1A, meaning command sent success, then it calls handle_frame to handle specic frame
//! ----------------------------------------------------------------------------
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

			//! Can be used to print received commands as ASCII text without having bugs in Nextion display due to double commands
			//for (uint8_t i = 0; i < local_frame_len; i++) {
			//	print_hex_byte_as_ascii(local_frame_buffer[i]);
			//} //can be commented out
			//usart_send_byte('\r');
			//usart_send_byte('\n');

			handle_frame(local_frame_buffer, local_frame_len); //handles inputs
		
		}
}

//! ----------------------------------------------------------------------------
//handles specific frame, meaning it starts what each frame command should do
//! ----------------------------------------------------------------------------
void handle_frame(const uint8_t *buffer, uint8_t len ) {

	uint8_t header = buffer[0];

	switch(header) {
		//! Won't work if USART is being used to send debugging messages or other things
		// case 0x00: usart_send_string((const uint8_t *)"EE 00: Invalid Instruction"); break;
		// case 0x1A: usart_send_string((const uint8_t*)"ERR 1A: invalid name\r\n"); break;
		// case 0x23: usart_send_string((const uint8_t*)"ERR 23: invalid attribute\r\n"); break;

		case 0x65: //! Touch event: 65 page id event
            if (len == 7) {
                uint8_t page  = buffer[1];
                uint8_t id    = buffer[2];
                uint8_t event = buffer[3]; // 1=press, 0=release

                if (event == 0) { // release only
                    if (page == PAGE0_ID && id == START_BUTTON_ID) {
                        app_on_start_pressed();
                    } else if (page == PAGE1_ID && id == RESTART_BUTTON_ID) {
                        app_on_restart_pressed();
                    }
                }
            }
            break;
		
		//! Number request from a get request to nextion - For each of the 4 requests
		case 0x71: 
			if (len >= 8) {
				uint32_t v = 0;
				// Nextion sends little-endian: b1, b2, b3, b4
				// Assemble the 4 bytes //Just how nextion works //Use chatgpt for debugging
				v  =  (uint32_t)buffer[1];
				v |= ((uint32_t)buffer[2] << 8);
				v |= ((uint32_t)buffer[3] << 16);
				v |= ((uint32_t)buffer[4] << 24); 
				
				//! Sends numebr to be store in correct variable depending on current Param_State
				app_on_nextion_number(v);
			}
			break;
		

		default:
				
        		break;
    }
}


void nextion_print_FFs() {
	for (int i = 0; i < 3; i++) usart_send_byte(0xFF);
}

void nextion_send_command(const char *cmd) {
    usart_send_string((const uint8_t *)cmd);
    nextion_print_FFs();
}


