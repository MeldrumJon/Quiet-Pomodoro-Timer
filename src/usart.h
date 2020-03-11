#ifndef USART_H
#define USART_H

#include <stdio.h>

/**
 * Write-only file stream for UART for use with <stdio.h>.
 */
extern FILE uart_stream;

/**
 * Sets the baud rate, enables the transmitter, but does not enable interrupts.
 */
void usart_init(void);

/**
 * Send a character over UART.  This function is more efficient than using printf.
 */
void usart_sendChar(char c);

/**
 * Send a string over UART.  This function is more efficient than using printf.
 */
void usart_sendString(char* str);

/**
 * Transmit a character over UART.
 *
 * @param data Character to trasmit.
 * @param stream Ignored. Included so that we can use this function to set up a file 
 *               stream
 * @return Always 0.  Returns an int.  Again included to match the definition of the file 
 *         stream function pointer
 */
int usart_put(char data, FILE* stream);

#endif