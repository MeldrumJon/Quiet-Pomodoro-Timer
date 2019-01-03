#include "wdt.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h> // only used for wdt_reset()


volatile static uint8_t g_timeout = 0;

ISR(WDT_vect) {
	g_timeout = 1;
}

uint8_t wdt_handleTimeout(void) {
	uint8_t timeout;

	cli();
	timeout = g_timeout;
	g_timeout = 0;
	sei();

	return timeout;
}
/**
	This code should be called at the beginning of main

	Turns off the WDT
**/
void wdt_off(void) {

	cli();
	/* Clear WDRF in MCUSR */
	MCUSR &= ~(0x1<<WDRF);
	/* Write logical one to WDCE and WDE */
	/* Keep old prescaler setting to prevent unintentional time-out */
	WDTCSR |= (0x1<<WDCE) | (0x1<<WDE);
	/* Turn off WDT */
	WDTCSR = 0x00;
	sei();

	return;
}

/**
	Sets the WDT to interrupt after wdt_time (where wdt_time is one of the WDP masks in .h)
**/
void wdt_start(uint8_t wdt_scaler_mask) {

	cli();
	wdt_reset();
	WDTCSR |= (0x1<<WDCE) | (0x1<<WDE);
	WDTCSR = (0x1<<WDIE) | wdt_scaler_mask;
	sei();
	
	return;
}
