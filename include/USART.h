#ifndef USART_H
#define USART_H

#include <system.h>

void init_usart();
void send_one_byte_data(uint8_t c);
void send_string(uint8_t *str);
void print_hex_byte_as_ascii(uint8_t hex_byte);

#endif