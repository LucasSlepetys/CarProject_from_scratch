#include "USART.h"
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

volatile static uint8_t usart_tx_busy = 1; //not busy

//Interrupt for transmitter:
//Whenver enabled: just finished transmitting
ISR(USART_TX_vect) {

    //transmission is not busy
    usart_tx_busy = 1;

}

void USART_INIT(uint32_t baud_rate) {

    // Normal speed
    UCSR0A &= ~(1 << U2X0);

    uint8_t speed = 16;

    uint32_t baud = (F_CPU / (speed * baud_rate)) - 1;

    uint16_t LOW_baud = (baud & 0b11111111);
    uint16_t HIGH_baud = ( (baud >> 8) &  0b11111111 );

    UBRR0L = LOW_baud;
    UBRR0H = HIGH_baud;

    //enables receiver and transmitter:
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);

    //enables receiver and transmitter interrupts:
    UCSR0B |= (1 << TXCIE0) | (1 << RXCIE0);

    //Reset C-register to default for 8 bit data frame:
    UCSR0C = 0b00000110;

}

void program_start() {
    const uint8_t start[] = "Program Start \n\r";
    usart_send_string(start);
}

//! ---------- Transmitting Helper Functions -----------

void usart_send_byte(uint8_t c){ 
    while(usart_tx_busy == 0); //wait until data is sending //usart is busy
    usart_tx_busy = 0; //usart is busy

    UDR0 = c;   //load data into buffer
}

void usart_send_array(uint8_t *c, uint16_t len) {
    for(uint16_t i = 0; i < len; i++) {
        usart_send_byte(c[i]);
    }
}

void usart_send_string(const uint8_t *str) {
    while(*str) {usart_send_byte(*str++);}
}

void print_hex_byte_as_ascii(uint8_t hex_byte){
	
	static const char Hex[] = "0123456789ABCDEF";
	
	usart_send_byte('[');
	usart_send_byte( Hex[ (hex_byte>>4) & 0b00001111 ] );
	usart_send_byte( Hex[ hex_byte & 0b00001111 ] );
	usart_send_byte(']');
}


//! -------------------------------



