#ifndef BTN_H
#define BTN_H

#include <stdbool.h>
#include <stdint.h>

void btn_enable(void);
void btn_disable(void);

void btn_tick(void);

bool btn_intrpted(void);
bool btn_pressed(void);
bool btn_longpressed(void);

#endif
