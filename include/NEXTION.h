#ifndef NEXTION_H
#define NEXTION_H

#include <system.h>
#include <USART.h>

#define FRAME_MAX 64

extern volatile uint8_t frame_buffer[FRAME_MAX];
extern volatile uint8_t frame_len;
extern volatile uint8_t frame_ready;

void collect_nextion_bytes(void);
void print_completed_frame_line(void);
void nextion_send_command(const char *cmd);

#endif