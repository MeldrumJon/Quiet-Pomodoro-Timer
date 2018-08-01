#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define DEBUG 1

#define LEDS_DATA_PIN 6
#define NUM_LEDS 12

#if DEBUG
#define MAX_TIME_U_SECONDS   60000000 // 1 min
#else
#define MAX_TIME_U_SECONDS 3600000000 // 1 hr
#endif
#define MAX_TICKS (MAX_TIME_U_SECONDS/TICK_U_SECONDS)
#define DFLT_TICKS_PER_LED (MAX_TICKS/NUM_LEDS)
#define LED_BRIGHTNESS_LVLS 4 // should be a power of two
#define TICKS_PER_FLASH_TOGGLE (1000000/TICK_U_SECONDS)

CRGB leds[NUM_LEDS];

static CRGB countdownColor = 0x330000;
static CRGB countupColor = 0x003300;
static CRGB selectColor = 0x000033;

enum timerFSM_state_t {
    IDLE_ST,
    COUNTDOWN_ST,
    ALERT_ST,
    COUNTUP_ST
};
static timerFSM_state_t currentState = IDLE_ST;

/** IDLE **/
static uint8_t indicators = 0; // Number of LEDs on indicates how much time the timer should run for.

static void reset() {
    indicators = 0;
    Serial.print("Indicators: ");
    Serial.println(indicators, DEC);
}
static void add() {
    ++indicators;
    Serial.print("Indicators: ");
    Serial.println(indicators, DEC);
}

/** CALIBRATE **/
static uint32_t ticksPerLED = DFLT_TICKS_PER_LED; // NUMBER of ticks it takes to turn off/on one LED
static uint32_t ticksPerLevel = DFLT_TICKS_PER_LED/LED_BRIGHTNESS_LVLS; // Number of ticks it takes to dim/brighten one LED

/** COUNTDOWN/COUNTUP **/
static uint32_t totalTicks = 0; // Number of ticks before we exit COUNTDOWN/COUNTUP
static uint8_t lvlCounter = 0;

static void initCountdown() {
    totalTicks = indicators*ticksPerLED;
    lvlCounter = LED_BRIGHTNESS_LVLS;
}
static void updateCountdown() {
    --lvlCounter;
    if (lvlCounter == 0) {
        lvlCounter = LED_BRIGHTNESS_LVLS;
        Serial.println("Powered off LED");
    }
    else {
        Serial.println("Dimmed LED");
    }
}
static void initCountup() {
    totalTicks = NUM_LEDS * ticksPerLED;
    lvlCounter = LED_BRIGHTNESS_LVLS;
}
static void updateCountup() {
    --lvlCounter;
    if (lvlCounter == 0) {
        lvlCounter = LED_BRIGHTNESS_LVLS;
        Serial.println("Powered on LED");
    }
    else {
        Serial.println("Brightened LED");
    }
}

void timerFSM_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN>(leds, NUM_LEDS);
    reset();
    currentState = IDLE_ST; // should be exactly the same as the static variables at the top
    return;
}

/** ALERT **/
static void toggleFlash() {
    static bool flashOn = false;
    if (flashOn) {
        Serial.println("Turn off alert!");
        flashOn = false;
    }
    else {
        Serial.println("Turn on alert!");
        flashOn = true;
    }
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

    if (btns & BTN_START_MASK && btns & BTN_ADD_MASK) {
        reset();
        currentState = IDLE_ST;
        return;
    }

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
            if (btns & BTN_ADD_MASK) {
                add();
            }
            else if (btns & BTN_START_MASK) {
                if (indicators > 0) {
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
            if (counter % ticksPerLevel == 0) {
                updateCountdown();
            }
            else if (counter >= totalTicks) {
                counter = 0;
                currentState = ALERT_ST;
            }
            break;
        case ALERT_ST:
            if (btns & BTN_START_MASK) {
                counter = 0;
                initCountup();
                currentState = COUNTUP_ST;
            }
            else if (counter >= TICKS_PER_FLASH_TOGGLE) {
                toggleFlash();
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