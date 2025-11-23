#include <NEXTION.h>

//! ----------------------------------------------------------------------------
//Handles frame
volatile uint8_t frame_buffer[FRAME_MAX]; //Should update every loop
volatile uint8_t frame_len = 0; //Should update every loop
volatile uint8_t frame_ready = 0; //Should update every loop
//! ----------------------------------------------------------------------------

volatile app_state_t   g_app_state;
volatile route_params_t g_route_params = {0};;
volatile param_state_t g_param_state;

volatile uint8_t g_start_pressed = 0; //Flag for when start button is pressed
volatile uint8_t g_restart_pressed = 0; //Flag for when restart button is pressed


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

void ui_update_page1_title(void)
{
    char cmd[80];

    // Build the Nextion command string.
    // Note the escaped quotes \" ... \"
    sprintf(cmd,
            "tTitle.txt=\"S1: %lu m in %lu s  S2: %lu m in %lu s\"",
            (unsigned long)g_route_params.d1,
            (unsigned long)g_route_params.t1,
            (unsigned long)g_route_params.d2,
            (unsigned long)g_route_params.t2);

    nextion_send_command(cmd);
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

                ui_handle_touch_event(page, id, event);
            }
            break;

		case 0x71: // Number return from "get x.val"
			if (len >= 8) {
				uint32_t v = 0;
				// Nextion sends little-endian: b1, b2, b3, b4
				v  =  (uint32_t)buffer[1];
				v |= ((uint32_t)buffer[2] << 8);
				v |= ((uint32_t)buffer[3] << 16);
				v |= ((uint32_t)buffer[4] << 24);

				ui_handle_number_response(v);
			}
			break;
		

		default:
				
        		break;
    }
}

// Handles every touch event from nextion
void ui_handle_touch_event(uint8_t page, uint8_t id, uint8_t event) {

    // Trigger only on RELEASE
    if (event != 0) return;

	//Start button
    if (page == PAGE0_ID && id == START_BUTTON_ID) {
        // Start button released on page0
        g_start_pressed = 1;

    }

	//Restart button
	if (page == PAGE1_ID && id == RESTART_BUTTON_ID) {
        g_restart_pressed = 1;
		g_start_pressed = 0;
    }

}

// Hanldes number response
// Called when a "get nX.val" response (0x71) arrives
void ui_handle_number_response(uint32_t value) {

    switch (g_param_state) {

        case PARAM_STATE_WAIT_D1:
            g_route_params.d1 = value;
            g_param_state = PARAM_STATE_REQ_T1;
            break;

        case PARAM_STATE_WAIT_T1:
            g_route_params.t1 = value;
            g_param_state = PARAM_STATE_REQ_D2;
            break;

        case PARAM_STATE_WAIT_D2:
            g_route_params.d2 = value;
            g_param_state = PARAM_STATE_REQ_T2;
            break;

        case PARAM_STATE_WAIT_T2:
            g_route_params.t2 = value;
            g_param_state = PARAM_STATE_DONE;
            break;

        default:
            // We got a number when we weren't expecting one. Ignore or log.
            break;
    }
}

void params_start_fetch(void) {
    g_param_state = PARAM_STATE_REQ_D1;
}

uint8_t params_is_done(void) {
    return (g_param_state == PARAM_STATE_DONE);
}



void params_update(void) {

    switch (g_param_state) {

        case PARAM_STATE_REQ_D1:
            // Ask Nextion for n0.val -> distance 1
            nextion_send_command("get page0.n0.val");
            g_param_state = PARAM_STATE_WAIT_D1;
            break;

        case PARAM_STATE_WAIT_D1:
            // Do nothing; we are waiting for 0x71 frame.
            // When it arrives, ui_handle_number_response() will move the state forward.
			
            break;

        case PARAM_STATE_REQ_T1:
            // Ask Nextion for n1.val -> time 1
            nextion_send_command("get n1.val");
            g_param_state = PARAM_STATE_WAIT_T1;
            break;

        case PARAM_STATE_WAIT_T1:
            // Waiting for number response
            break;

        case PARAM_STATE_REQ_D2:
            // Ask Nextion for n2.val -> distance 2
            nextion_send_command("get n2.val");
            g_param_state = PARAM_STATE_WAIT_D2;
            break;

        case PARAM_STATE_WAIT_D2:
            // Waiting for number response
            break;

        case PARAM_STATE_REQ_T2:
            // Ask Nextion for n3.val -> time 2
            nextion_send_command("get n3.val");
            g_param_state = PARAM_STATE_WAIT_T2;
            break;

        case PARAM_STATE_WAIT_T2:
            // Waiting for number response
            break;

        case PARAM_STATE_DONE:
        case PARAM_STATE_IDLE:
        default:
            // Nothing to do here
            break;
    }
}

void app_update(void) {

    switch (g_app_state) {

        case APP_STATE_IDLE:
            // Waiting on page0 for Start button
            if (g_start_pressed) {
                g_start_pressed = 0;
				
                // Start reading n0..n3
                params_start_fetch();
                g_app_state = APP_STATE_FETCH_PARAMS;
            }
            break;

        case APP_STATE_FETCH_PARAMS:
            // Step through parameter reading
            params_update();

            if (params_is_done()) {
                g_app_state = APP_STATE_CALCULATE;
            }
            break;

        case APP_STATE_CALCULATE:

            // Here g_route_params.{d1,t1,d2,t2} are ready

            // Example: do your math (ticks, speeds, etc.)
            // calc_from_route_params(&g_route_params);    // your function

            // Page1 is already active (Nextion did page change)
            // Set "waiting..." -> "In progress"

			nextion_send_command("page page1");
			ui_update_page1_title();
            nextion_send_command("tStatus.txt=\"In progress\"");

            // Start your car / motor controller
            // motor_start_route(&g_route_params);         // your function

            g_app_state = APP_STATE_RUNNING;
            break;

        case APP_STATE_RUNNING:
            // Update your motor control + runtime display

            // 1) Update motor control (non-blocking)
            // motor_update();

            // 2) Update Nextion fields (distance, speed, time) on page1
            // ui_update_runtime_fields();  // you will implement this

            // 3) Check if route is done
            // if (motor_is_finished()) {
            //     nextion_send_command("tStatus.txt=\"Completed\"");
            //     g_app_state = APP_STATE_COMPLETED;
            // }
			nextion_send_command("tStatus.txt=\"Completed\"");
			g_app_state = APP_STATE_COMPLETED;
            break;

        case APP_STATE_COMPLETED:
            // Wait for restart button
            if (g_restart_pressed) {
                g_restart_pressed = 0;

                // Go back to page0 (if not already done by Nextion)
                nextion_send_command("page page0");

                // Reset states
                g_param_state = PARAM_STATE_IDLE;
                g_app_state   = APP_STATE_IDLE;
            }
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


