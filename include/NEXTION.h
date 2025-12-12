#ifndef NEXTION_H
#define NEXTION_H

#include <system.h>
#include <USART.h>
#include <stdint.h>

// -----------------------------------------------------------------------------
// Constants / IDs
// -----------------------------------------------------------------------------
#define FRAME_MAX           64

#define PAGE0_ID            0
#define PAGE1_ID            1
#define START_BUTTON_ID     15
#define RESTART_BUTTON_ID   18


// Raw frame buffering (filled by USART RX ISR, consumed by nextion_handle_frame)
extern volatile uint8_t frame_buffer[FRAME_MAX];
extern volatile uint8_t frame_len;
extern volatile uint8_t frame_ready;

// -----------------------------------------------------------------------------
// API functions
// -----------------------------------------------------------------------------

// Called frequently in main loop: copies finished frame and dispatches it
void nextion_handle_frame(void);

// Internal frame handler (called by nextion_handle_frame)
void handle_frame(const uint8_t *buffer, uint8_t len);

// Send helpers
void nextion_print_FFs(void);
void nextion_send_command(const char *cmd);

#endif // NEXTION_H
