#include "states.h"
#include <avr/sleep.h>
#include "../encoder.h"
#include "../wdt.h"
#include "../usart.h"

#define POWER_OFF_TICKS 3

static uint8_t leds = 0;
static uint8_t power_off_ticks = 0;

void state_idle_action(void) {
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	printf("WOKEUP\r\n");
	int_fast8_t value = encoder_delta();
	if (value) {
		leds += value;
		power_off_ticks = 0;
		wdt_start(WDT_8S_MSK);
		printf("LEDs: %d\r\n", leds);
	}
	if (wdt_handleTimeout()) {
		++power_off_ticks;
		if (power_off_ticks >= POWER_OFF_TICKS) {
			printf("Powering down...\r\n");
			wdt_off();
		}
	}
}

void state_idle_init(void) {
	fsm_action = state_idle_action;
	printf("Entering IDLE state\r\n");
	power_off_ticks = 0;
	wdt_start(WDT_8S_MSK);
}