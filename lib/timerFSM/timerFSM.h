#ifndef TIMERFSM_H_
#define TIMERFSM_H_

#include <stdint.h>

void timerFSM_init();

void timerFSM_tick(uint8_t btns);

#endif