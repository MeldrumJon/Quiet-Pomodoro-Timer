// #define CONTROLLER_DEBUG

#include "controller.h"
#include <util/delay.h>
#ifdef CONTROLLER_DEBUG
#include <stdio.h>
#endif
#include "disp.h"
#include "btn.h"
#include "timer.h"
#include "encoder.h"
#include "led.h"

static enum controller_st_t {
	INIT_ST,
	OFF_ST,
	WAKE_UP_ST,
	SET_TIME_ST,
	SET_TIME_DIM_ST,
	COUNTDOWN_ST,
	COUNTDOWN_DIM_ST,
	COUNTDOWN_FINISHED_ST,
	ALERT_ST
} currentState = INIT_ST;

#define DEFAULT_MINUTES 25
#define MAX_MINUTES 60
#define MIN_MINUTES 0

#define WAKE_TIMEOUT TIMER_TICKS(0.25)
#define UI_TIMEOUT TIMER_TICKS(15)
#define MIN_TIMEOUT TIMER_TICKS(1)
#define ALERT_OFF_TIMEOUT TIMER_TICKS(60)
#define ALERT_TIMEOUT TIMER_TICKS(1)

static uint16_t wake_to_cnt;
static uint16_t ui_to_cnt;
static uint16_t min_cnt;
static uint16_t alert_cnt;

static int8_t minutes = DEFAULT_MINUTES;
static int8_t countdown_mins;
static uint8_t alert_state = 0;

// Save power by disabling peripherals.
static void powerdown(void) {
	disp_off();
	encoder_disable(); // Don't disable buttons, we need them to wake up!
}

static void powerup(void) {
	disp_on();
	encoder_enable();
}

// static uint16_t mins;
// static uint16_t ui_to_cnt;

#ifdef CONTROLLER_DEBUG
// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint(void) {
	static controller_st_t previousState;
	static bool firstPass = true;
	// Only print the message if:
	// 1. This the first pass and the value for previousState is unknown.
	// 2. previousState != currentState - this prevents reprinting the same state name over and over.
	if (previousState != currentState || firstPass) {
    	firstPass = false;                // previousState will be defined, firstPass is false.
    	previousState = currentState;     // keep track of the last state that you were in.
		switch (currentState) {
			case INIT_ST:
				printf("INIT_ST\r\n");
				break;
			case OFF_ST:
				printf("OFF_ST\r\n");
				break;
			case WAKE_UP_ST:
				printf("WAKE_UP_ST\r\n");
				break;
			case SET_TIME_ST:
				printf("SET_TIME_ST\r\n");
				break;
			case SET_TIME_DIM_ST:
				printf("SET_TIME_DIM_ST\r\n");
				break;
			case COUNTDOWN_ST:
				printf("COUNTDOWN_ST\r\n");
				break;
			case COUNTDOWN_DIM_ST:
				printf("COUNTDOWN_DIM_ST\r\n");
				break;
			case COUNTDOWN_FINISHED_ST:
				printf("COUNTDOWN_FINISHED_ST\r\n");
				break;
			case ALERT_ST:
				printf("ALERT_ST\r\n");
				break;
		}
	}
}
#else
void debugStatePrint(void) {}
#endif

void controller_wakeup(void) {
	if (currentState == OFF_ST) {
		wake_to_cnt = 0;
		currentState = WAKE_UP_ST;
	}
	
	debugStatePrint();
	return;
}

uint_fast8_t controller_isOff(void) {
	return currentState == OFF_ST;
}

void controller_tick(void) {
	int_fast8_t delta = encoder_delta();

	if (btn_longpress_flag) {
		powerdown();
		currentState = OFF_ST;
		return;
	}

	// State action
	switch (currentState) {
		case INIT_ST:
			break;
		case OFF_ST:
			// No ticks will happen while in this state.
			break;
		case WAKE_UP_ST:
			++wake_to_cnt;
			break;
		case SET_TIME_ST:
			++ui_to_cnt;
			break;
		case SET_TIME_DIM_ST:
			++ui_to_cnt;
			break;
		case COUNTDOWN_ST:
			++min_cnt;
			++ui_to_cnt;
			break;
		case COUNTDOWN_DIM_ST:
			++min_cnt;
			break;
		case COUNTDOWN_FINISHED_ST:
			++alert_cnt;
			break;
		case ALERT_ST:
			++min_cnt;
			++alert_cnt;
			led_off();
			break;
		default:
			currentState = INIT_ST;
			break;
	}
	
	// State transition
	switch (currentState) {
		case INIT_ST:
			minutes = DEFAULT_MINUTES;
			ui_to_cnt = 0;
			disp_setContrast(DISP_CONTRAST_ON);
			disp_drawTime(minutes, DISP_BLUE);
			currentState = SET_TIME_ST;
			break;
		case OFF_ST:
			// Transition happens in controller_wakeup() function
			break;
		case WAKE_UP_ST:
			if (btn_press_flag) {
				powerup();
				ui_to_cnt = 0;
				disp_setContrast(DISP_CONTRAST_ON);
				disp_drawTime(minutes, DISP_BLUE);
				currentState = SET_TIME_ST;
			}
			else if (wake_to_cnt >= WAKE_TIMEOUT) {
				currentState = OFF_ST;
			}
			break;
		case SET_TIME_ST:
			if (btn_press_flag) {
				min_cnt = 0;
				ui_to_cnt = 0;
				disp_redrawCircles(minutes, DISP_RED);
				countdown_mins = minutes;
				currentState = COUNTDOWN_ST;
			}
			else if (delta) {
				minutes += delta;
				if (minutes > MAX_MINUTES) { minutes = MAX_MINUTES; }
				if (minutes < MIN_MINUTES) { minutes = MIN_MINUTES; }
				disp_diffTime(minutes, DISP_BLUE);
				ui_to_cnt = 0;
				currentState = SET_TIME_ST;
			}
			else if (ui_to_cnt >= UI_TIMEOUT) {
				disp_setContrast(DISP_CONTRAST_DIM);
				ui_to_cnt = 0;
				currentState = SET_TIME_DIM_ST;
			}
			break;
		case SET_TIME_DIM_ST:
			if (btn_press_flag) {
				min_cnt = 0;
				ui_to_cnt = 0;
				disp_setContrast(DISP_CONTRAST_ON);
				disp_redrawCircles(minutes, DISP_RED);
				countdown_mins = minutes;
				currentState = COUNTDOWN_ST;
			}
			else if (delta) {
				minutes += delta;
				if (minutes > MAX_MINUTES) { minutes = MAX_MINUTES; }
				if (minutes < MIN_MINUTES) { minutes = MIN_MINUTES; }
				disp_diffTime(minutes, DISP_BLUE);
				ui_to_cnt = 0;
				disp_setContrast(DISP_CONTRAST_ON);
				currentState = SET_TIME_ST;
			}
			else if (ui_to_cnt >= UI_TIMEOUT) {
				powerdown();
				currentState = OFF_ST;
			}
			break;
		case COUNTDOWN_ST:
			if (min_cnt >= MIN_TIMEOUT) {
				--countdown_mins;
				min_cnt = 0;
				disp_diffTime(countdown_mins, DISP_RED);
			}
			
			if (countdown_mins <= 0 || btn_press_flag) {
				min_cnt = 0;
				alert_cnt = 0;
				currentState = COUNTDOWN_FINISHED_ST;
			}
			else if (ui_to_cnt >= UI_TIMEOUT) {
				disp_setContrast(DISP_CONTRAST_DIM);
				currentState = COUNTDOWN_DIM_ST;
			}
			break;
		case COUNTDOWN_DIM_ST:
			if (min_cnt >= MIN_TIMEOUT) {
				--countdown_mins;
				min_cnt = 0;
				disp_diffTime(countdown_mins, DISP_RED);
			}
			
			if (countdown_mins <= 0 || btn_press_flag) {
				alert_cnt = 0;
				currentState = COUNTDOWN_FINISHED_ST;
			}
			else if (delta) {
				disp_setContrast(DISP_CONTRAST_ON);
				ui_to_cnt = 0;
				currentState = COUNTDOWN_ST;
			}
			break;
		case COUNTDOWN_FINISHED_ST:
			if (alert_cnt >= ALERT_TIMEOUT || btn_press_flag) {
				min_cnt = 0;
				alert_cnt = 0;
				disp_clear();
				disp_setContrast(DISP_CONTRAST_ON);
				alert_state = !(0);
				currentState = ALERT_ST;
			}
			else if (delta) {
				disp_setContrast(DISP_CONTRAST_ON);
			}
			break;
		case ALERT_ST:
			if (btn_press_flag) {
				ui_to_cnt = 0;
				disp_drawTime(minutes, DISP_BLUE);
				led_off();
				currentState = SET_TIME_ST;
			}
			else if (min_cnt >= ALERT_OFF_TIMEOUT) {
				led_off();
				powerdown();
				currentState = OFF_ST;
			}
			else if (alert_cnt >= ALERT_TIMEOUT) {
				alert_cnt = 0;
				disp_alert(alert_state);
				alert_state = !alert_state;
				if (!alert_state) {
				    led_on();
                    _delay_ms(8); // LEDs on for 5ms - 10ms are noticeable
                    led_off();
                }
				currentState = ALERT_ST;
			}
			break;
		default:
			currentState = INIT_ST;
			break;
	}
	
	debugStatePrint();
	return;
}
