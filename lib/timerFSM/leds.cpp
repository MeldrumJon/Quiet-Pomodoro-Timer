#include "leds.h"
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

CRGB leds[NUM_LEDS];

#define SELECT_COLOR b
#define ALERT_COLOR r
#define COUNTDOWN_COLOR r
#define COUNTUP_COLOR g

#define NUM_BRIGHT_PREFS 17
#define NUM_BRIGHT_LVLS 8

#define MAX_BRIGHT_IDX 7
#define MIN_BRIGHT_IDX 0

const uint8_t gamma_lut[NUM_BRIGHT_PREFS][NUM_BRIGHT_LVLS] = {
	{ 1, 1, 2, 2, 3, 3, 4, 4},
	{ 1, 2, 3, 4, 5, 6, 7, 8},
	{ 1, 2, 3, 4, 6, 8, 10, 12},
	{ 1, 2, 4, 6, 9, 11, 14, 18},
	{ 1, 2, 4, 7, 10, 15, 20, 26},
	{ 1, 3, 5, 9, 14, 20, 27, 35},
	{ 1, 4, 7, 12, 18, 26, 35, 46},
	{ 1, 4, 9, 15, 23, 33, 44, 58},
	{ 1, 3, 8, 15, 25, 38, 54, 73},
	{ 1, 4, 9, 18, 30, 46, 65, 89},
	{ 1, 4, 11, 22, 36, 55, 79, 107},
	{ 1, 5, 13, 26, 43, 65, 93, 126},
	{ 1, 6, 16, 30, 50, 76, 109, 148},
	{ 1, 7, 18, 35, 58, 89, 127, 172},
	{ 2, 8, 21, 40, 67, 102, 145, 197},
	{ 2, 9, 24, 46, 76, 116, 166, 225},
	{ 2, 11, 27, 52, 87, 132, 188, 255}
};

static uint8_t alertGammaPref_idx = 9;
static uint8_t timerGammaPref_idx = 6;

void leds_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear(false); // Colors should already be 0
}

void leds_clearRange(leds_color_t color, uint8_t start_idx, uint8_t lt_idx) {
    switch (color) {
        case SELECT:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].SELECT_COLOR = 0;
            }
            break;
        case ALERT:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].ALERT_COLOR = 0;
            }
            break;
        case COUNTDOWN:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].COUNTDOWN_COLOR = 0;
            }
            break;
        case COUNTUP:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].COUNTUP_COLOR = 0;
            }
            break;
    }
    FastLED.show();
}

void leds_clear() {
    FastLED.clear(true);
}

void leds_fillRange(leds_color_t color, uint8_t start_idx, uint8_t lt_idx) {
    switch (color) {
        case SELECT:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].SELECT_COLOR = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
            }
            break;
        case ALERT:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].ALERT_COLOR = gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
            }
            break;
        case COUNTDOWN:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].COUNTDOWN_COLOR = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
            }
            break;
        case COUNTUP:
            for (uint8_t i = start_idx; i < lt_idx; ++i) {
                leds[i].COUNTUP_COLOR = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
            }
            break;
    }
    FastLED.show();
}

void leds_fill(leds_color_t color, uint8_t num) {
    leds_fillRange(color, 0, num);
}

void leds_brightenAlert() {
    alertGammaPref_idx = (alertGammaPref_idx + 1) % NUM_BRIGHT_PREFS;
}

void leds_darkenAlert() {
    alertGammaPref_idx = (alertGammaPref_idx==0) ? NUM_BRIGHT_PREFS - 1 : alertGammaPref_idx - 1;
}

void leds_brightenTimer() {
    timerGammaPref_idx = (timerGammaPref_idx + 1) % NUM_BRIGHT_PREFS;
}

void leds_darkenTimer() {
    timerGammaPref_idx = (timerGammaPref_idx==0) ? NUM_BRIGHT_PREFS - 1 : timerGammaPref_idx - 1;
}