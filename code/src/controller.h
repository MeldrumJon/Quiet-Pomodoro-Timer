#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>

void controller_wakeup(void);
uint_fast8_t controller_isOff(void);
void controller_tick(void);

#endif