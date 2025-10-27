#ifndef NEXTION_H
#define NEXTION_H

#include <system.h>
#include <USART.h>

#define FRAME_MAX 64

extern volatile uint8_t frame_buffer[FRAME_MAX];
extern volatile uint8_t frame_len;
extern volatile uint8_t frame_ready;

void nextion_handle_frame(void);
void handle_frame(const uint8_t *buffer, uint8_t len);
void nextion_print_FFs();
void nextion_send_command(const char *cmd);

#endif