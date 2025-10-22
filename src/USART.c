#include <USART.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//9600 baud - 16Mhz, normal mode
void init_usart() {

    //Due to potential bugs:
    UCSR0A &= ~(1 << U2X0);   // disable double-speed mode (normal speed)

    UBRR0H = 0;
    UBRR0L = 103;  

    UCSR0B = (1<<RXEN0)|(1<<TXEN0); // enable RX & TX
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); // 8 data, no parity, 1 stop
}

//sends 8-bit data to serial port (a.k.a one char)
void send_one_byte_data(uint8_t c) {

    while(!(UCSR0A & (1 << UDRE0))); //Wait until buffer is empty
    UDR0 = c; //Adds char to buffer

}

//sends a string until char: \0 (which is False)
void send_string(uint8_t *str) {

    while(*str) { //while True or not \0
        send_one_byte_data(*str++); //increments pointer by 1
    }

}

void print_hex_byte_as_ascii(uint8_t hex_byte){
	
	static const char Hex[] = "0123456789ABCDEF";
	
	send_one_byte_data('[');
	send_one_byte_data( Hex[ (hex_byte>>4) & 0b00001111 ] );
	send_one_byte_data( Hex[ hex_byte & 0b00001111 ] );
	send_one_byte_data(']');
}


