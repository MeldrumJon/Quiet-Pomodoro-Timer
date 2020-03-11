#include "btn.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "timer.h"

#define DEBOUNCE_TICKS TIMER_TICKS(0.050)
#define LONGPRESS_TICKS TIMER_TICKS(2)

static enum btn_st_t {
	LOW_ST,
	WAIT_ST,
	HIGH_ST,
	LONG_ST
} currentState = LOW_ST;

static uint16_t counter;

volatile static bool btn_intrpt_flag = false;
static bool btn_press_flag = false;
static bool btn_longpress_flag = false;

#define BTN_DMASK 0x04
#define INT0_EN_MASK 0x01

ISR(INT0_vect, ISR_BLOCK) {
	btn_intrpt_flag = true;
}

void btn_enable(void) {
	sei();
	DDRD &= ~(BTN_DMASK); // Set pin as input
	PORTD |= BTN_DMASK; // Activate pull-up
	EICRA |= 0x02; // Falling edge interrupt
	EIMSK |= INT0_EN_MASK; // Set interrupt mask on INT0
	cli();
	return;
}

void btn_disable(void) {
	sei();
	DDRD |= BTN_DMASK; // Set pin as out
	PORTD &= ~(BTN_DMASK); // Drive low
	EIMSK &= ~(INT0_EN_MASK); // Disable interrupt
	cli();
	return;
}

void btn_tick(void) {
	uint8_t btn = !(PIND & BTN_DMASK); // Get button state (active-low)

	// State action
	switch (currentState) {
		case LOW_ST:
			break;
		case WAIT_ST:
			++counter;
			break;
		case HIGH_ST:
			++counter;
			btn_press_flag = 0;
			break;
		case LONG_ST:
			btn_longpress_flag = 0;
			break;
		default:
			break;
	}
	
	// State transition
	switch (currentState) {
		case LOW_ST:
			if (btn) {
				counter = 0;
				currentState = WAIT_ST;
			}
			break;
		case WAIT_ST:
			if (!btn) {
				currentState = LOW_ST;
			}
			else if (counter >= DEBOUNCE_TICKS) {
				counter = 0;
				btn_press_flag = !(0);
				currentState = HIGH_ST;
			}
			break;
		case HIGH_ST:
			if (!btn) {
				currentState = LOW_ST;
			}
			else if (counter >= LONGPRESS_TICKS) {
				btn_longpress_flag = !(0);
				currentState = LONG_ST;
			}
			break;
		case LONG_ST:
			if (!btn) {
				currentState = LOW_ST;
			}
			break;
		default:
			break;
	}
	
	return;
}

bool btn_intrpted(void) {
    cli();
    bool result = btn_intrpt_flag;
    btn_intrpt_flag = false;
    sei();
    return result;
}

bool btn_pressed(void) {
    cli();
    bool result = btn_press_flag;
    btn_press_flag = false;
    sei();
    return result;
}

bool btn_longpressed(void) {
    cli();
    bool result = btn_longpress_flag;
    btn_longpress_flag = false;
    sei();
    return result;
}
