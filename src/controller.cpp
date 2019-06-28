#include "controller.h"
#include <stdio.h>

static enum controller_st_t {
	INIT_ST,
	OFF_ST,
	WAKE_UP_ST,
	SET_TIME_ST,
	SET_TIME_DIM_ST,
	COUNTDOWN_ST,
	COUNTDOWN_DIM_ST,
	ALERT_ST
} currentState = INIT_ST;

// static uint16_t mins;
// static uint16_t ui_to_cnt;

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
			case ALERT_ST:
				printf("ALERT_ST\r\n");
				break;
		}
	}
}

void controller_tick(void) {
	debugStatePrint();

	// State action
	switch (currentState) {
		case INIT_ST:
			break;
		case OFF_ST:
			break;
		case WAKE_UP_ST:
			break;
		case SET_TIME_ST:
			break;
		case SET_TIME_DIM_ST:
			break;
		case COUNTDOWN_ST:
			break;
		case COUNTDOWN_DIM_ST:
			break;
		case ALERT_ST:
			break;
		default:
			printf("Error\r\n");
			break;
	}
	
	// State transition
	switch (currentState) {
		case INIT_ST:
// 			ui_to_cnt = 0;
			
			break;
		case OFF_ST:
			break;
		case WAKE_UP_ST:
			break;
		case SET_TIME_ST:
			break;
		case SET_TIME_DIM_ST:
			break;
		case COUNTDOWN_ST:
			break;
		case COUNTDOWN_DIM_ST:
			break;
		case ALERT_ST:
			break;
		default:
			printf("Error\r\n");
			break;
	}
	
	return;
}