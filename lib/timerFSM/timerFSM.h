#ifndef TIMERFSM_H_
#define TIMERFSM_H_

#include <stdint.h>

void timerFSM_init();

//void timerFSM_tick(uint8_t btns, int_fast8_t change);

void (*timerFSM_tick)(uint_fast8_t, int_fast8_t);

void idle_st(uint_fast8_t btns, int_fast8_t change);
void select_st(uint_fast8_t btns, int_fast8_t change);
void countdown_st(uint_fast8_t btns, int_fast8_t change);
void alert_st(uint_fast8_t btns, int_fast8_t change);
void countup_st(uint_fast8_t btns, int_fast8_t change);
void setTimerBrightness_st(uint_fast8_t btns, int_fast8_t change);
void setAlertBrightness_st(uint_fast8_t btns, int_fast8_t change);
void setTicksPerLEDWait_st(uint_fast8_t btns, int_fast8_t change);
void setTicksPerLED_st(uint_fast8_t btns, int_fast8_t change);


#endif