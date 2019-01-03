#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

#define BAUD 38400UL
#define UBRR_VAL ((F_CPU/(16*BAUD))-1)

FILE uart_stream = FDEV_SETUP_STREAM(usart_put, NULL, _FDEV_SETUP_WRITE);

void usart_init() {

	/* Set baud rate */
	UBRR0H = (unsigned char) (UBRR_VAL >> 8);
	UBRR0L = (unsigned char) UBRR_VAL;
	
	// Enable transmitter (w/out interrupts)
	UCSR0B = (1 << TXEN0);
	//UCSR0C = (1 << USBS0) | (0x3 << UCSZ00); // 8 bit data, 2 stop bits
	UCSR0C = (0x3 << UCSZ00); // 8 bit data, 1 stop bits
}

/**
	Transmit a character over UART.
	
	To support file streams (be able to use this with stdout):
	stream parameter ignored.
	returns an int (0)
**/
int usart_put(char data, FILE *stream) {

	while ( !(UCSR0A & (1 << UDRE0)) ); // Wait until transmit buffer ready to receive data
	UDR0 = data;
	
	// Wait for transmit to complete (optional)
	// Use these when using sleep mode (going to sleep before transmit is complete will corrupt data)
	while ( !(UCSR0A & (1 << TXC0)) );
	UCSR0A |= (1 << TXC0);
	
	return 0;
}
