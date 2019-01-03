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
	power_usart0_enable(); // Setup UART
	usart_init();
	stdout = &uart_stream;
	
	encoder_init(); // Set up encoder interrupts
	
	// Test: We are ready!
	printf("\r\nHello world!\r\n"); // Does the UART work?

	uint16_t count = 0; // Count of WDT interrupts
	wdt_start(WDT_1S); // Interrupt every 4S
	while(1) {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		sleep_mode();
		wdt_start(WDT_8S); // Interrupt every 4S
		printf("WOKE UP!\r\n");
		int_fast8_t value = encoder_delta();
		if (value != 0) {
			printf("Delta: %d\r\n", value);
		}
		if (wdt_handleTimeout()) {
			printf("Timer Expired %d\r\n", count++);
		}
	}
}
