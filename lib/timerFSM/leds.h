#include <stdint.h>

enum leds_color_t {
    SELECT,
    ALERT,
    COUNTDOWN,
    COUNTUP
};

void leds_init();
void leds_clearRange(leds_color_t color, uint8_t start_idx, uint8_t lt_idx);
void leds_clear();
void leds_fillRange(leds_color_t color, uint8_t start_idx, uint8_t lt_idx);
void leds_fill(leds_color_t color, uint8_t num);

void leds_brightenAlert();
void leds_darkenAlert();
void leds_brightenTimer() ;
void leds_darkenTimer();