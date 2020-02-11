#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void encoder_enable(void);
void encoder_disable(void);

int_fast8_t encoder_delta(void);

#endif