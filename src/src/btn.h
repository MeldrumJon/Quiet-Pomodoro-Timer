#ifndef BTN_H
#define BTN_H

#include <stdint.h>

void btn_enable(void);
void btn_disable(void);
void btn_tick(void);

extern uint8_t btn_press_flag;
extern uint8_t btn_longpress_flag;
extern volatile uint8_t btn_intrpt_flag;

#endif