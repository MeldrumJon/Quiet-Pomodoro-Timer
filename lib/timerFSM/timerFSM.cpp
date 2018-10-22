#define DEBUG 1
#define SHORT 0

#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#if DEBUG
#define FASTLED_ALLOW_INTERRUPTS 0
#endif
#include <FastLED.h> 

/* LEDs */

CRGB lights[NUM_LEDS];

/* Timing Calculations */
#if SHORT
#define MAX_TIME_U_SECONDS   60000000 // 1 min (for testing)
#else
#define MAX_TIME_U_SECONDS 3600000000 // 1 hr
#endif

#define MAX_TICKS (MAX_TIME_U_SECONDS/TICK_U_SECONDS)
#define DFLT_TICKS_PER_LED 15141 // (MAX_TICKS/NUM_LEDS)
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

static void select_init() {
    FastLED.clear(true);
    accumulated = 0;
    for (uint8_t i = adjusted_idx; i < NUM_LEDS; ++i) {
        lights[i].SELECT_COLOR = TIMER_MAX_BRIGHTNESS;
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
            lights[i].SELECT_COLOR = TIMER_MAX_BRIGHTNESS;
        }
    }
    else if (change < 0) {
        for (uint8_t i = adjusted_idx; i < new_adjusted; ++i) {
            lights[i].SELECT_COLOR = 0;
        }
    }
    adjusted_idx = new_adjusted;
    FastLED.show();
}

static void countdown_init() {
    totalTicks = numSelected * ticksPerLED;
    brightLvl_idx = MAX_BRIGHT_IDX;
    for (uint8_t i = adjusted_idx; i < NUM_LEDS; ++i) {
        lights[i].b = 0; // transitions from IDLE state
        lights[i].r = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
    }
    FastLED.show();
    count_idx = adjusted_idx; // start dimming at the proper LED
}
static void countdown_update() {
    if (brightLvl_idx == 0) {
        lights[count_idx].r = 0;
        ++count_idx;
        brightLvl_idx = MAX_BRIGHT_IDX;
    }
    else {
        --brightLvl_idx;
        lights[count_idx].r = gamma_lut[timerGammaPref_idx][brightLvl_idx];
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
    lights[count_idx].g = gamma_lut[timerGammaPref_idx][brightLvl_idx];
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
                lights[i].r = (i%2) ? gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX] : 0;
            }
        }
        else {
            for (uint8_t i = 0; i < NUM_LEDS; ++i) {
                lights[i].r = (i%2) ? 0 : gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
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
            lights[i].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
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
            lights[i].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
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
            lights[i].r = gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
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
            lights[i].r = gamma_lut[alertGammaPref_idx][MAX_BRIGHT_IDX];
        }
    }
    #if DEBUG
    Serial.println(alertGammaPref_idx);
    #endif
    FastLED.show();
}

static void ticksPerLedWait_init() {
    //FastLED.clear(true); // Not necessary, since the LEDs should be off after SELECT_ST
    lights[0].r = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
    FastLED.show();
}

static void ticksPerLed_init() {
    lights[0].r = 0;
    lights[0].g = gamma_lut[timerGammaPref_idx][MAX_BRIGHT_IDX];
    FastLED.show();
}

static void setBrightness_save() {

}

static void ticksPerLed_save(uint_fast16_t count) {
    ticksPerLED = count;
    ticksPerLevel = count/NUM_BRIGHT_LVLS;

    #if DEBUG
    Serial.print("ticksPerLED: ");
    Serial.println(ticksPerLED, DEC);
    #endif
}



/** FSM **/
// # if DEBUG
// static void debugStatePrint(bool printState) {
//     static timerFSM_state_t previousState = SELECT_ST;
//     static bool firstPass = true;
//     if (previousState != timerFSM_tick || firstPass) { // only print after a state transition
//         firstPass = false;
//         switch(timerFSM_tick) {
//         case IDLE_ST:
//             if (printState) { Serial.println("timerFSM: IDLE_ST"); }
//             break;
//         case SELECT_ST:
//             if (printState) { Serial.println("timerFSM: SELECT_ST"); }
//             break;
//         case COUNTDOWN_ST:
//             if (printState) { Serial.println("timerFSM: COUNTDOWN_ST"); }
//             break;
//         case ALERT_ST:
//             if (printState) { Serial.println("timerFSM: ALERT_ST"); }
//             break;
//         case COUNTUP_ST:
//             if (printState) { Serial.println("timerFSM: COUNTUP_ST"); }
//             break;
//         case SET_TIMER_BRIGHTNESS:
//             if (printState) { Serial.println("timerFSM: SET_TIMER_BRIGHTNESS"); }
//             break;
//         case SET_ALERT_BRIGHTNESS:
//             if (printState) { Serial.println("timerFSM: SET_ALERT_BRIGHTNESS"); }
//             break;
//         case SET_TICKS_PER_LED_WAIT:
//             if (printState) { Serial.println("timerFSM: SET_TICKS_PER_LED_WAIT"); }
//             break;
//         case SET_TICKS_PER_LED:
//             if (printState) { Serial.println("timerFSM: SET_TICKS_PER_LED"); }
//             break;
//         default: // error, we forgot to include a state
//             Serial.println("timerFSM debugStatePrint: hit default");
//             break;
//         }
//     }
//     previousState = timerFSM_tick;
//     return;
// }
// #endif

static void reset() {
    numSelected = 0;
    adjusted_idx = NUM_LEDS;
    accumulated = 0;
    FastLED.clear(true); // turn off the LEDs and write 0s to the LED array
    timerFSM_tick = select_st;
}

void timerFSM_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(lights, NUM_LEDS);
    reset();
    return;
}

static uint_fast16_t counter = 0;

void select_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action         
    if (change != 0) {
        counter = 0;
        select_changeBy(change);
    }
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK) {
        if (numSelected > 0) {
            counter = 0;
            countdown_init();
            timerFSM_tick = countdown_st;
        }
        else {
            counter = 0;
            countup_init();
            timerFSM_tick = countup_st;
        }
    }
    else if (accumulated >= NUM_PREF_DETENTS) {
        counter = 0;
        initBrightnessCount();
        timerFSM_tick = setTimerBrightness_st;
    }
    else if (accumulated <= -(NUM_PREF_DETENTS)) {
        counter = 0;
        ticksPerLedWait_init();
        timerFSM_tick = setTicksPerLEDWait_st;
    }
    else if (counter >= TIMEOUT_TICKS) {
        FastLED.clear(true);
        timerFSM_tick = idle_st;
    }
}

void countdown_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (counter % ticksPerLevel == 0) {
        countdown_update();
    }
    else if (counter >= totalTicks) {
        counter = 0;
        timerFSM_tick = alert_st;
    }
}

void alert_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK || counter >= ALARM_TIMEOUT_TICKS) {
        counter = 0;
        countup_init();
        timerFSM_tick = countup_st;
    }
    else if ((counter % TICKS_PER_FLASH_TOGGLE) == 0) {
        alert_toggle();
    }
}

void countup_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK || counter >= totalTicks) {
        counter = 0;
        select_init();
        timerFSM_tick = select_st;
    }
    else if (counter % ticksPerLevel == 0) {
        countup_update();
    }
}

void setTimerBrightness_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    if (change != 0) {
        counter = 0;
        brightnessCountUpdate(change);
    }
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK) {
        counter = 0;
        initBrightnessAlert();
        timerFSM_tick = setAlertBrightness_st;
    }
    else if (counter >= TIMEOUT_TICKS) {
        counter = 0;
        reset(); // moves to SELECT_ST
    }
}

void setAlertBrightness_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    if (change != 0) {
        counter = 0;
        brightnessAlertUpdate(change);
    }
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (counter >= TIMEOUT_TICKS
        || (btns & BTN_START_MASK)) {
        setBrightness_save();
        counter = 0;
        reset(); // moves to SELECT_ST
    }
}

void setTicksPerLEDWait_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK) {
        counter = 0;
        ticksPerLed_init();
        timerFSM_tick = setTicksPerLED_st;
    }
    else if (counter >= TIMEOUT_TICKS) {
        counter = 0;
        reset(); // moves to SELECT_ST
    }
}

void setTicksPerLED_st(uint_fast8_t btns, int_fast8_t change) {
    // State Action
    ++counter;

    // State Update
    if (btns & BTN_RESET_MASK) {
        counter = 0;
        reset();
        return;
    }

    if (btns & BTN_START_MASK) {
        ticksPerLed_save(counter);
        reset(); // moves to SELECT_ST
    }
}