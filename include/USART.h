#ifndef USART_H
#define USART_H

#include <system.h>

void USART_INIT(uint32_t baud_rate);
void program_start();
void usart_send_byte(uint8_t c);
void usart_send_array(uint8_t *c, uint16_t len);
void usart_send_string(const uint8_t *str);
void print_hex_byte_as_ascii(uint8_t hex_byte);


#endif