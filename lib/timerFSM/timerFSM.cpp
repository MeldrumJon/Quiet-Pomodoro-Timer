#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define DEBUG 0
#define SHORT 1

/* Timer FSM */
enum timerFSM_state_t {
    IDLE_ST,
    SELECT_ST,
    COUNTDOWN_ST,
    ALERT_ST,
    COUNTUP_ST,
    SET_BRIGHTNESS_COUNT,
    SET_BRIGHTNESS_ALERT
};
static timerFSM_state_t currentState = SELECT_ST;

/* LEDs */
#define LEDS_DATA_PIN 6
#define NUM_LEDS 12

CRGB leds[NUM_LEDS];

/* Timing Calculations */
#if SHORT
#define MAX_TIME_U_SECONDS   60000000 // 1 min (for testing)
#else
#define MAX_TIME_U_SECONDS 3600000000 // 1 hr
#endif

#define MAX_TICKS (MAX_TIME_U_SECONDS/TICK_U_SECONDS)
#define DFLT_TICKS_PER_LED (MAX_TICKS/NUM_LEDS)
#define TICKS_PER_FLASH_TOGGLE (1000000/TICK_U_SECONDS) // About 1s (to avoid epilepsy, avoid >= 3Hz)
#define TIMEOUT_TICKS (15000000/TICK_U_SECONDS) // About 15s
#define ALARM_TIMEOUT_TICKS (300000000/TICK_U_SECONDS) // About 5min

/* Colors */
#define NUM_BRIGHT_PREFS 17
#define NUM_BRIGHT_LVLS 8 // Best if it's 2^n

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

#define SELECT_COLOR b
#define ALERT_COLOR r
#define COUNTDOWN_COLOR r
#define COUNTUP_COLOR g

#define TIMER_MAX_BRIGHTNESS gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX]
#define ALERT_MAX_BRIGHTNESS gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX]

/** **/
#define NUM_PREF_DETENTS 60
static int8_t alertGammaPref_idx = 9;
static int8_t timerGammaPref_idx = 6;

/* IDLE */
static int_fast8_t numSelected = 0; // Number of LEDs on indicates how much time the timer should run for.
static int_fast8_t adjusted_idx = NUM_LEDS; // NUM_LEDS - numSelected
static int8_t accumulated = 0;
/** CALIBRATE **/
static uint32_t ticksPerLED = DFLT_TICKS_PER_LED; // Number of ticks it takes to turn off/on one LED
static uint32_t ticksPerLevel = DFLT_TICKS_PER_LED/NUM_BRIGHT_LVLS; // Number of ticks it takes to dim/brighten one LED
/** COUNTDOWN/COUNTUP **/
static uint_fast8_t count_idx = 0;
static uint32_t totalTicks = 0; // Number of ticks before we exit COUNTDOWN/COUNTUP
static uint8_t brightLvl_idx = 0;

static void reset() {
    numSelected = 0;
    adjusted_idx = NUM_LEDS;
    accumulated = 0;
    FastLED.clear(true); // turn off the LEDs and write 0s to the LED array
    currentState = SELECT_ST;
}

static void select_clear() {
    numSelected = 0;
    adjusted_idx = NUM_LEDS;
    accumulated = 0;
    FastLED.clear(true); // turn off the LEDs and write 0s to the LED array
}

static void select_init() {
    accumulated = 0;
    for (uint8_t i = adjusted_idx; i < NUM_LEDS; ++i) {
        leds[i].SELECT_COLOR = TIMER_MAX_BRIGHTNESS;
    }
    FastLED.show();
}

static void select_changeBy(int_fast8_t change) {
    numSelected = numSelected + change;
    if (numSelected < 0) {
        numSelected = 0;
        accumulated = accumulated + change;
    }
    else if (numSelected > NUM_LEDS) {
        numSelected = NUM_LEDS;
        accumulated = accumulated + change;
    }
    else {
        accumulated = 0;
    }
    int_fast8_t new_adjusted = NUM_LEDS - numSelected;
    if (change >= 0) {
        for (uint8_t i = new_adjusted; i < adjusted_idx; ++i) {
            leds[i].SELECT_COLOR = TIMER_MAX_BRIGHTNESS;
        }
    }
    else if (change < 0) {
        for (uint8_t i = adjusted_idx; i < new_adjusted; ++i) {
            leds[i].SELECT_COLOR = 0;
        }
    }
    adjusted_idx = new_adjusted;
    FastLED.show();
}

static void countdown_init() {
    totalTicks = numSelected * ticksPerLED;
    brightLvl_idx = MAX_BRIGHT_IDX;
    for (uint8_t i = adjusted_idx; i < NUM_LEDS; ++i) {
        leds[i].b = 0; // transitions from IDLE state
        leds[i].r = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
    }
    FastLED.show();
    count_idx = adjusted_idx; // start dimming at the proper LED
}
static void countdown_update() {
    if (brightLvl_idx == 0) {
        leds[count_idx].r = 0;
        ++count_idx;
        brightLvl_idx = MAX_BRIGHT_IDX;
    }
    else {
        --brightLvl_idx;
        leds[count_idx].r = gamma_lut[timerGammaPref_idx][brightLvl_idx];
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
static void countup_init() {
    totalTicks = NUM_LEDS * ticksPerLED;
    brightLvl_idx = MIN_BRIGHT_IDX;
    FastLED.clear(true);
    count_idx = 0;
}
static void countup_update() {
    leds[count_idx].g = gamma_lut[timerGammaPref_idx][brightLvl_idx];
    brightLvl_idx = (brightLvl_idx + 1) % NUM_BRIGHT_LVLS;
    if (brightLvl_idx == 0) {
        ++count_idx;
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
static void alert_toggle() {
    static bool flashOn = true;
    if (flashOn) {
        static bool altPattern = false;
        if (altPattern) {
            for (uint8_t i = 0; i < NUM_LEDS; ++i) {
                leds[i].r = (i%2) ? gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX] : 0;
            }
        }
        else {
            for (uint8_t i = 0; i < NUM_LEDS; ++i) {
                leds[i].r = (i%2) ? 0 : gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
            }
        }
        altPattern = !altPattern;
        FastLED.show();
    }
    else {
        FastLED.clear(true);
    }
    flashOn = !flashOn;
    #if DEBUG
    if (!flashOn) {
        Serial.println("Turn on alert!");
    }
    else {
        Serial.println("Turn off alert!");
    }
    #endif
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
static void brightnessCountUpdate(int_fast8_t change) {
    if (change >= 0) {
        timerGammaPref_idx = (timerGammaPref_idx + change) % NUM_BRIGHT_PREFS;
    }
    else if (change < 0) {
        timerGammaPref_idx = timerGammaPref_idx + change;
        if (timerGammaPref_idx < 0) {
            timerGammaPref_idx = NUM_BRIGHT_PREFS + timerGammaPref_idx;
        }
    }
    for (uint8_t i = 0; i < NUM_LEDS; ++i) {
        if (!(i%BRIGHT_ADJUST_PATTERN)) {
            leds[i].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    #if DEBUG
    Serial.println(timerGammaPref_idx);
    #endif
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
static void brightnessAlertUpdate(int_fast8_t change) {
    if (change >= 0) {
        alertGammaPref_idx = (alertGammaPref_idx + change) % NUM_BRIGHT_PREFS;
    }
    else if (change < 0) {
        alertGammaPref_idx = alertGammaPref_idx + change;
        if (alertGammaPref_idx < 0) {
            alertGammaPref_idx = NUM_BRIGHT_PREFS + alertGammaPref_idx;
        }
    }
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

void timerFSM_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);
    reset();
    return;
}

/** FSM **/
# if DEBUG
static void debugStatePrint(bool printState) {
    static timerFSM_state_t previousState = SELECT_ST;
    static bool firstPass = true;
    if (previousState != currentState || firstPass) { // only print after a state transition
        firstPass = false;
        switch(currentState) {
        case IDLE_ST:
            if (printState) { Serial.println("timerFSM: IDLE_ST"); }
            break;
        case SELECT_ST:
            if (printState) { Serial.println("timerFSM: SELECT_ST"); }
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

void timerFSM_tick(uint_fast8_t btns, int_fast8_t change) {
    static uint_fast16_t counter = 0;

    #if DEBUG
    debugStatePrint(true);
    #endif

    // State Action
    switch(currentState) {
        case IDLE_ST:
            if (btns != 0 || change != 0) {
                counter = 0;
                select_init();
                currentState = SELECT_ST;
            }
            else {
                return;
            }
            // Don't break.  Any changes in IDLE_ST should move us to SELECT_ST and then happen.
        case SELECT_ST:
            if (change != 0) {
                counter = 0;
                select_changeBy(change);
                break;
            }
            ++counter;
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
            if (change != 0) {
                counter = 0;
                brightnessCountUpdate(change);
                break;
            }
            ++counter;
            break;
        case SET_BRIGHTNESS_ALERT:
            if (change != 0) {
                counter = 0;
                brightnessAlertUpdate(change);
                break;
            }
            ++counter;
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state update hit default");
            #endif
            break;
    }

    // State Transitions
    if (btns & BTN_RESET_MASK) {
        reset();
    }

    switch(currentState) {
        case IDLE_ST:
        case SELECT_ST:
            if (btns & BTN_START_MASK) {
                if (numSelected > 0) {
                    counter = 0;
                    countdown_init();
                    currentState = COUNTDOWN_ST;
                }
                else {
                    counter = 0;
                    countup_init();
                    currentState = COUNTUP_ST;
                }
            }
            else if (accumulated >= NUM_PREF_DETENTS) {
                counter = 0;
                initBrightnessCount();
                currentState = SET_BRIGHTNESS_COUNT;
            }
            else if (counter >= TIMEOUT_TICKS) {
                FastLED.clear(true);
                currentState = IDLE_ST;
            }
            break;
        case COUNTDOWN_ST:
            if (counter % ticksPerLevel == 0) {
                countdown_update();
            }
            else if (counter >= totalTicks) {
                counter = 0;
                currentState = ALERT_ST;
            }
            break;
        case ALERT_ST:
            if (btns & BTN_START_MASK || counter >= ALARM_TIMEOUT_TICKS) {
                counter = 0;
                countup_init();
                currentState = COUNTUP_ST;
            }
            else if (counter >= TICKS_PER_FLASH_TOGGLE) {
                alert_toggle();
                counter = 0;
            }
            break;
        case COUNTUP_ST:
            if (btns & BTN_START_MASK) {
                FastLED.clear(true);
                select_init();
                currentState = SELECT_ST;
            }
            else if (counter % ticksPerLevel == 0) {
                countup_update();
            }
            else if (counter >= totalTicks) {
                FastLED.clear(true);
                select_init();
                currentState = SELECT_ST;
            }
            break;
        case SET_BRIGHTNESS_COUNT:
            if (btns & BTN_START_MASK) {
                counter = 0;
                currentState = SET_BRIGHTNESS_ALERT;
                initBrightnessAlert();
            }
            else if (counter >= TIMEOUT_TICKS) {
                select_clear();
                currentState = SELECT_ST;
            }
            break;
        case SET_BRIGHTNESS_ALERT:
            if (counter >= TIMEOUT_TICKS
                || (btns & BTN_START_MASK)) {
                select_clear();
                currentState = SELECT_ST;
            }
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state transition hit default");
            #endif
            break;
    }
}