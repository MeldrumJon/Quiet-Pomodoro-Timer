#include <stdio.h>

extern FILE uart_stream;

void usart_init();

int usart_put(char data, FILE* stream);
