#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

// Sets the baud rate.  See Table 20-6 in the ATmega328p datasheet for values with low
// error.
#define BAUD 38400UL
#define UBRR_VAL ((F_CPU/(16*BAUD))-1) // See Table 20-1 for baud rate register equations

// Wait until a transmit is complete before exiting a USART function.
// This is useful if you are planning on going to sleep right after exiting the function.
// (waiting for the transmit to complete will not corrupt the last 2 bytes sent)
#define WAIT_UNTIL_TRANSMIT_COMPLETE 1

// Filestream for use with stdio.
FILE uart_stream;

/**
 * Waits until the transmit buffer can receive data, then sends the byte.
 */
static inline void send(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until TX buffer ready to receive data
	UDR0 = c; // Send the data.
}

/**
 * Waits for a transmit to complete.
 * This is useful if you are planning on turning off the UART after returning from a USART
 * function (if you went to sleep before the transmit was complete, the last byte would
 * be corrupted).
 */
static inline void wait_for_complete(void) {
	loop_until_bit_is_set (UCSR0A, TXC0); // Wait until transmit is complete.
	UCSR0A |= _BV(TXC0); // Clear the transmit complete bit (toggle-on-write)
}

/**
 * Sets the baud rate, enables the transmitter, but does not enable interrupts.
 */
void usart_init(void) {
	fdev_setup_stream(&uart_stream, usart_put, NULL, _FDEV_SETUP_WRITE);

	// Set baud rate
	UBRR0H = (unsigned char) (UBRR_VAL >> 8);
	UBRR0L = (unsigned char) UBRR_VAL;
	
	// Enable transmitter (w/out interrupts)
	UCSR0B = _BV(TXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8 bit data, 1 stop bit
	
	return;
}

/**
 * Send a character over UART.  This function is more efficient than using printf.
 */
void usart_sendChar(char c) {
	send(c);

	#if WAIT_UNTIL_TRANSMIT_COMPLETE
	wait_for_complete();
	#endif
	
	return;
}

/**
 * Send a string over UART.  This function is more efficient than using printf.
 */
void usart_sendString(char* str) {
	while ((*str) != '\0') { // Send until we hit NULL terminator.
		send(*(str++));
	}

	#if WAIT_UNTIL_TRANSMIT_COMPLETE
	wait_for_complete();
	#endif
	
	return;
}

/**
 * Transmit a character over UART.
 *
 * @param data Character to trasmit.
 * @param stream Ignored. Included so that we can use this function to set up a file 
 *               stream
 * @return Always 0.  Returns an int.  Again included to match the definition of the file 
 *         stream function pointer
 */
int usart_put(char data, FILE *stream) {
	send(data);

	#if WAIT_UNTIL_TRANSMIT_COMPLETE
	wait_for_complete();
	#endif
	
	return 0;
}