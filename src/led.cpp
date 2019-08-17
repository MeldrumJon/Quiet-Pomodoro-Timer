#include "led.h"
#include <avr/io.h>

#define LED_DMASK 0x40

void led_init(void) {
	DDRD |= LED_DMASK; // Output
	PORTD &= ~(LED_DMASK); // Off
	return;
}

void led_on(void) {
	PORTD |= LED_DMASK;
	return;
}

void led_off(void) {
	PORTD &= ~(LED_DMASK);
	return;
}