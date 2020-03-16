#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define TIMER_PRESCALER 1024
#define TIMER_OC 75
#define TIMER_TICKS(seconds) ((seconds*F_CPU)/TIMER_PRESCALER/TIMER_OC)

void timer_enable(void);
void timer_disable(void);
bool timer_intrpted(void);

#endif

