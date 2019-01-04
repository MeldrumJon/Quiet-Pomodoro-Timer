#define DEBUG

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "usart.h"
#include "encoder.h"
#include "wdt.h"
#include "fsm/states.h"

#define LED_PIN_MASK (0x2)

int main ()
{
	wdt_off(); // Turn off WDT (should be done early to avoid infinite reset loops)
	// Power saving
	power_all_disable(); // Disable everything we don't use
	DDRB = 0x00; // Set direction to input on all pins
	DDRC = 0x00;
	DDRD = 0x00;
	PORTB = 0xFF; // Enable pull-ups on pins 
	PORTC = 0xFF;
	PORTD = 0xFF;
	sei();
	// Setup
	encoder_init(); // Set up encoder interrupts
	power_usart0_enable(); // Setup UART
	usart_init();
	stdout = &uart_stream;
	usart_sendChar('\r'); // Does the UART work?
	usart_sendChar('\n');
	usart_sendString("Hello ");
	printf("world!\r\n");

	// Start state machine execution.
	state_idle_init();
	while(1) {
		(*fsm_action)();
	}
}
