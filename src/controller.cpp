#include "controller.h"
#include "flags.h"

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

void controller_tick(void) {

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
			FLAGS_SET(FLAG_RESET);
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
			FLAGS_SET(FLAG_RESET);
			break;
	}
	
	return;
}