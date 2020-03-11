#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

bool controller_inDeepSleep(void);

void controller_wake(void);

void controller_press(void);

void controller_longpress(void);

void controller_rotate(int8_t delta);

void controller_tick(void);

#endif
