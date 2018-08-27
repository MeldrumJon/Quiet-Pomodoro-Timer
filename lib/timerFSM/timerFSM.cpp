#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define DEBUG 1

// LEDs
#define LEDS_DATA_PIN 6
#define NUM_LEDS 12

// Timing Calculations
#if DEBUG
#define MAX_TIME_U_SECONDS   60000000 // 1 min
#else
#define MAX_TIME_U_SECONDS 3600000000 // 1 hr
#endif
#define MAX_TICKS (MAX_TIME_U_SECONDS/TICK_U_SECONDS)
#define DFLT_TICKS_PER_LED (MAX_TICKS/NUM_LEDS)
#define TICKS_PER_FLASH_TOGGLE (1000000/TICK_U_SECONDS) // About 1s
#define SETTING_TIMEOUT_TICKS (60000000/TICK_U_SECONDS) // About 1min

// Fade brightness
#define NUM_BRIGHT_PREFS 17
#define NUM_BRIGHT_LVLS 8 // Should be 2^n

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

CRGB leds[NUM_LEDS];

enum timerFSM_state_t {
    IDLE_ST,
    COUNTDOWN_ST,
    ALERT_ST,
    COUNTUP_ST,
    SET_BRIGHTNESS_COUNT,
    SET_BRIGHTNESS_ALERT
};
static timerFSM_state_t currentState = IDLE_ST;

/** IDLE **/
static uint8_t led_idx = 0; // Number of LEDs on indicates how much time the timer should run for.

static void reset() {
    led_idx = 0;
    FastLED.clear(true); // turn off the LEDs and write 0s to the LED array
    #if DEBUG
    Serial.print("led_idx: ");
    Serial.println(led_idx, DEC);
    #endif
}
static void add() { // TODO: handle overflow
    if (led_idx < NUM_LEDS) {
        leds[led_idx].b = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
        ++led_idx;
        FastLED.show();
    }
    else {
        reset();
    }
    #if DEBUG
    Serial.print("led_idx: ");
    Serial.println(led_idx, DEC);
    #endif
}

/** CALIBRATE **/
static uint32_t ticksPerLED = DFLT_TICKS_PER_LED; // NUMBER of ticks it takes to turn off/on one LED
static uint32_t ticksPerLevel = DFLT_TICKS_PER_LED/NUM_BRIGHT_LVLS; // Number of ticks it takes to dim/brighten one LED

/** COUNTDOWN/COUNTUP **/
static uint32_t totalTicks = 0; // Number of ticks before we exit COUNTDOWN/COUNTUP
static uint8_t brightLvl_idx = 0;

static void initCountdown() {
    totalTicks = led_idx*ticksPerLED;
    brightLvl_idx = MAX_BRIGHT_IDX;
    //memset(&leds, countdownShades[0], sizeof(CRGB)*led_idx); // Doesn't work.  CRGB probably contains much more data than we actually want to transfer anyway.
    for (uint8_t i = 0; i < led_idx; ++i) {
        leds[i].b = 0;
        leds[i].r = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
    }
    FastLED.show();
    --led_idx; // start dimming at the proper LED
}
static void updateCountdown() {
    if (brightLvl_idx == 0) {
        leds[led_idx].r = 0;
        --led_idx;
        brightLvl_idx = MAX_BRIGHT_IDX;
    }
    else {
        --brightLvl_idx;
        leds[led_idx].r = gamma_lut[timerGammaPref_idx][brightLvl_idx];
    }
    FastLED.show();

    #if DEBUG
    if (brightLvl_idx == MAX_BRIGHT_IDX)  {
        Serial.println("Powered off LED");
    }
    else {
        Serial.println("Dimmed LED");
    }
    #endif
}
static void initCountup() {
    totalTicks = NUM_LEDS * ticksPerLED;
    brightLvl_idx = MIN_BRIGHT_IDX;
    FastLED.clear(true);
    led_idx = 0;
}
static void updateCountup() {
    leds[led_idx].g = gamma_lut[timerGammaPref_idx][brightLvl_idx];
    brightLvl_idx = (brightLvl_idx + 1) % NUM_BRIGHT_LVLS;
    if (brightLvl_idx == 0) {
        ++led_idx;
    }
    FastLED.show();

    #if DEBUG
    if (brightLvl_idx == 0)  {
        Serial.println("Powered on LED");
    }
    else {
        Serial.println("Brightened LED");
    }
    #endif

}

/** ALERT **/
static void toggleAlert() {
    static bool flashOn = false;
    if (flashOn) {
        for (uint8_t i = 0; i < NUM_LEDS; ++i) {
            leds[i].r = (i%2) ? gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX] : 0;
        }
        Serial.println("Turn off alert!");
        flashOn = false;
    }
    else {
        for (uint8_t i = 0; i < NUM_LEDS; ++i) {
            leds[i].r = (i%2) ? 0 : gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
        }
        Serial.println("Turn on alert!");
        flashOn = true;
    }
    FastLED.show();
}

/** SETTINGS **/
#define BRIGHT_ADJUST_PATTERN 2

static void initBrightnessCount() {
    FastLED.clear(true);
    for (uint8_t i = 0; i < NUM_LEDS; ++i) {
        if (!(i%BRIGHT_ADJUST_PATTERN)) {
            leds[i].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    FastLED.show();
}
static void brightnessCountUpdate(bool add) {
    timerGammaPref_idx = (add) ? (timerGammaPref_idx + 1) % NUM_BRIGHT_PREFS
        : (timerGammaPref_idx==0) ? NUM_BRIGHT_PREFS-1 : timerGammaPref_idx - 1;
    for (uint8_t i = 0; i < NUM_LEDS; ++i) {
        if (!(i%BRIGHT_ADJUST_PATTERN)) {
            leds[i].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    FastLED.show();
}

static void initBrightnessAlert() {
    FastLED.clear(true);
    for (uint8_t i = 0; i < NUM_LEDS; ++i) {
        if (!(i%BRIGHT_ADJUST_PATTERN)) {
            leds[i].r = gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    FastLED.show();
}
static void brightnessAlertUpdate(bool add) {
    alertGammaPref_idx = (add) ? (alertGammaPref_idx + 1) % NUM_BRIGHT_PREFS
        : (alertGammaPref_idx==0) ? NUM_BRIGHT_PREFS-1 : alertGammaPref_idx - 1;
    for (uint8_t i = 0; i < NUM_LEDS; ++i) {
        if (!(i%BRIGHT_ADJUST_PATTERN)) {
            leds[i].r = gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    #if DEBUG
    Serial.println(alertGammaPref_idx);
    #endif
    FastLED.show();
}


static void saveBrightnessSettings() {
    // uint8_t shadeDecrement = scale8(color8, SHADES_SCALAR);
    // shades8[NUM_BRIGHT_LVLS] = color8;
    // for (uint8_t i = NUM_BRIGHT_LVLS-1; i > 0; --i) { // 1, 2, 3, 4
    //     shades8[i] = shades8[i+1] - shadeDecrement;
    // }
    // colorHalf8 = color8/2;

    // #if DEBUG
    // Serial.print("color8: ");
    // Serial.println(color8, HEX);
    // Serial.print("colorHalf8: ");
    // Serial.println(colorHalf8, HEX);
    // Serial.print("alert8: ");
    // Serial.println(alert8, HEX);
    // Serial.print("shade8: [ ");
    // for (uint8_t i = 0; i < NUM_BRIGHT_LVLS+1; ++i) {
    //     Serial.print(shades8[i]);
    //     Serial.print(" ");
    // }
    // Serial.println(" ]");
    // #endif
}

void timerFSM_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);
    reset();
    currentState = IDLE_ST; // should be exactly the same as the static variables at the top
    return;
}

/** FSM **/
# if DEBUG
static void debugStatePrint(bool printState) {
    static timerFSM_state_t previousState = IDLE_ST;
    static bool firstPass = true;
    if (previousState != currentState || firstPass) { // only print after a state transition
        firstPass = false;
        switch(currentState) {
        case IDLE_ST:
            if (printState) { Serial.println("timerFSM: IDLE_ST"); }
            break;
        case COUNTDOWN_ST:
            if (printState) { Serial.println("timerFSM: COUNTDOWN_ST"); }
            break;
        case ALERT_ST:
            if (printState) { Serial.println("timerFSM: ALERT_ST"); }
            break;
        case COUNTUP_ST:
            if (printState) { Serial.println("timerFSM: COUNTUP_ST"); }
            break;
        case SET_BRIGHTNESS_COUNT:
            if (printState) { Serial.println("timerFSM: SET_BRIGHTNESS_COUNT"); }
            break;
        case SET_BRIGHTNESS_ALERT:
            if (printState) { Serial.println("timerFSM: SET_BRIGHTNESS_ALERT"); }
            break;
        default: // error, we forgot to include a state
            Serial.println("timerFSM debugStatePrint: hit default");
            break;
        }
    }
    previousState = currentState;
    return;
}
#endif

void timerFSM_tick(uint8_t btns) {
    static uint16_t counter = 0;

    #if DEBUG
    debugStatePrint(true);
    #endif

    switch(currentState) {
        case IDLE_ST:
            break;
        case COUNTDOWN_ST:
            ++counter;
            break;
        case ALERT_ST:
            ++counter;
            break;
        case COUNTUP_ST:
            ++counter;
            break;
        case SET_BRIGHTNESS_COUNT:
            break;
        case SET_BRIGHTNESS_ALERT:
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state update hit default");
            #endif
            break;
    }

    switch(currentState) {
        case IDLE_ST:
            if (btns & BTN_START_MASK && btns & BTN_ADD_MASK) {
                reset();
                currentState = IDLE_ST;
            }
            else if (btns & BTN_ADD_MASK) {
                add();
            }
            else if (btns & BTN_START_MASK) {
                if (led_idx > 0) {
                    counter = 0;
                    initCountdown();
                    currentState = COUNTDOWN_ST;
                }
                else {
                    counter = 0;
                    initCountup();
                    currentState = COUNTUP_ST;
                }
            }
            else if (btns & LONG_PRESS_ADD_MASK) {
                counter = 0;
                initBrightnessCount();
                currentState = SET_BRIGHTNESS_COUNT;
            }
            break;
        case COUNTDOWN_ST:
            if (btns & BTN_START_MASK && btns & BTN_ADD_MASK) {
                reset();
                currentState = IDLE_ST;
            }
            else if (counter % ticksPerLevel == 0) {
                updateCountdown();
            }
            else if (counter >= totalTicks) {
                counter = 0;
                currentState = ALERT_ST;
            }
            break;
        case ALERT_ST:
            if (btns & BTN_START_MASK && btns & BTN_ADD_MASK) {
                reset();
                currentState = IDLE_ST;
            }
            else if (btns & BTN_START_MASK) {
                counter = 0;
                initCountup();
                currentState = COUNTUP_ST;
            }
            else if (counter >= TICKS_PER_FLASH_TOGGLE) {
                toggleAlert();
                counter = 0;
            }
            break;
        case COUNTUP_ST:
            if (btns & BTN_START_MASK) {
                reset();
                currentState = IDLE_ST;
            }
            else if (counter % ticksPerLevel == 0) {
                updateCountup();
            }
            else if (counter >= totalTicks) {
                reset();
                currentState = IDLE_ST;
            }
            break;
        case SET_BRIGHTNESS_COUNT:
            if (btns & BTN_START_MASK && btns & BTN_ADD_MASK) {
                counter = 0;
                currentState = SET_BRIGHTNESS_ALERT;
                initBrightnessAlert();
            }
            else if (counter >= SETTING_TIMEOUT_TICKS) {
                saveBrightnessSettings();
                reset();
                currentState = IDLE_ST;
            }
            else if (btns & BTN_ADD_MASK) {
                counter = 0;
                brightnessCountUpdate(true); // add to the brightness
            }
            else if (btns & BTN_START_MASK) {
                counter = 0;
                brightnessCountUpdate(false); // subtract from the brightness
            }
            break;
        case SET_BRIGHTNESS_ALERT:
            if (counter >= SETTING_TIMEOUT_TICKS
                || (btns & BTN_START_MASK && btns & BTN_ADD_MASK)) {
                saveBrightnessSettings();
                reset();
                currentState = IDLE_ST;
            }
            else if (btns & BTN_ADD_MASK) {
                counter = 0;
                brightnessAlertUpdate(true); // add to the brightness
            }
            else if (btns & BTN_START_MASK) {
                counter = 0;
                brightnessAlertUpdate(false); // subtract from the brightness
            }
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state transition hit default");
            #endif
            break;
    }
}