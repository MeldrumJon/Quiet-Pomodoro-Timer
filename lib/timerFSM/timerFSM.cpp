#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define DEBUG 1

#define LEDS_DATA_PIN 6
#define NUM_LEDS 12

#define BRIGHTNESS_INC_VALUE 5
#define NUM_BRIGHTNESS_LVLS 4 // should be a power of two
#define NUM_TOTAL_SHADES 5 // should be a 2^n+1

#if DEBUG
#define MAX_TIME_U_SECONDS   60000000 // 1 min
#else
#define MAX_TIME_U_SECONDS 3600000000 // 1 hr
#endif
#define MAX_TICKS (MAX_TIME_U_SECONDS/TICK_U_SECONDS)
#define DFLT_TICKS_PER_LED (MAX_TICKS/NUM_LEDS)
#define TICKS_PER_FLASH_TOGGLE (1000000/TICK_U_SECONDS) // About 1s

CRGB leds[NUM_LEDS];

// Colors are GRB
static uint8_t color8 = 0x44;
static uint8_t colorHalf8 = 0x22;
static uint8_t shades8[NUM_TOTAL_SHADES] = {0x00, 0x11, 0x22, 0x33, 0x44};
static const CRGB OFF_COLOR = 0x000000;

enum timerFSM_state_t {
    IDLE_ST,
    COUNTDOWN_ST,
    ALERT_ST,
    COUNTUP_ST
};
static timerFSM_state_t currentState = IDLE_ST;

/** IDLE **/
static uint8_t ledIdx = 0; // Number of LEDs on indicates how much time the timer should run for.

static void reset() {
    ledIdx = 0;
    FastLED.clear(true); // turn off the LEDs and write 0s to the LED array
    #if DEBUG
    Serial.print("ledIdx: ");
    Serial.println(ledIdx, DEC);
    #endif
}
static void add() { // TODO: handle overflow
    if (ledIdx < NUM_LEDS) {
        leds[ledIdx].b = color8;
        ++ledIdx;
        FastLED.show();
    }
    else {
        reset();
    }
    #if DEBUG
    Serial.print("ledIdx: ");
    Serial.println(ledIdx, DEC);
    #endif
}

/** CALIBRATE **/
static uint32_t ticksPerLED = DFLT_TICKS_PER_LED; // NUMBER of ticks it takes to turn off/on one LED
static uint32_t ticksPerLevel = DFLT_TICKS_PER_LED/NUM_BRIGHTNESS_LVLS; // Number of ticks it takes to dim/brighten one LED

/** COUNTDOWN/COUNTUP **/
static uint32_t totalTicks = 0; // Number of ticks before we exit COUNTDOWN/COUNTUP
static uint8_t shadeIdx = 0;

static void initCountdown() {
    totalTicks = ledIdx*ticksPerLED;
    shadeIdx = NUM_BRIGHTNESS_LVLS;
    //memset(&leds, countdownShades[0], sizeof(CRGB)*ledIdx); // Doesn't work.  CRGB probably contains much more data than we actually want to transfer anyway.
    for (uint8_t i = 0; i < ledIdx; ++i) {
        leds[i].b = 0;
        leds[i].r = color8;
    }
    FastLED.show();
    --ledIdx; // start dimming at the proper LED
}
static void updateCountdown() {
    --shadeIdx;
    leds[ledIdx].r = shades8[shadeIdx];
    if (shadeIdx == 0) {
        --ledIdx;
        shadeIdx = NUM_BRIGHTNESS_LVLS;
    }
    FastLED.show();

    #if DEBUG
    if (shadeIdx == NUM_BRIGHTNESS_LVLS)  {
        Serial.println("Powered off LED");
    }
    else {
        Serial.println("Dimmed LED");
    }
    #endif
}
static void initCountup() {
    totalTicks = NUM_LEDS * ticksPerLED;
    shadeIdx = NUM_BRIGHTNESS_LVLS;
    FastLED.clear(true);
    ledIdx = 0;
}
static void updateCountup() {
    --shadeIdx;
    leds[ledIdx].g = shades8[NUM_BRIGHTNESS_LVLS - shadeIdx];
    if (shadeIdx == 0) {
        ++ledIdx;
        shadeIdx = NUM_BRIGHTNESS_LVLS;
    }
    FastLED.show();

    #if DEBUG
    if (shadeIdx == NUM_BRIGHTNESS_LVLS)  {
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
            leds[i].r = (i%2) ? color8 : 0;
        }
        Serial.println("Turn off alert!");
        flashOn = false;
    }
    else {
        for (uint8_t i = 0; i < NUM_LEDS; ++i) {
            leds[i].r = (i%2) ? 0 : color8;
        }
        Serial.println("Turn on alert!");
        flashOn = true;
    }
    FastLED.show();
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
                if (ledIdx > 0) {
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
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state transition hit default");
            #endif
            break;
    }
}