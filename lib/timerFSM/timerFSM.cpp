#include <Arduino.h>
#include <timerFSM.h>
#include <ringTimeCommon.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define DEBUG 1

#define LEDS_DATA_PIN 6
#define LEDS_NUM 12
CRGB leds[LEDS_NUM];

static CRGB countdownColor = 0x330000;
static CRGB countupColor = 0x003300;
static CRGB selectColor = 0x000033;

enum timerFSM_state_t {
    IDLE_ST,
    COUNTDOWN,
    ALERT,
    COUNTUP
};
static timerFSM_state_t currentState = IDLE_ST;

void timerFSM_init() {
    FastLED.addLeds<WS2812, LEDS_DATA_PIN>(leds, LEDS_NUM);
    FastLED.show();
    currentState = IDLE_ST; // should be exactly the same as the static variables at the top
    return;
}

// FSM
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
        case COUNTDOWN:
            if (printState) { Serial.println("timerFSM: COUNTDOWN"); }
            break;
        case ALERT:
            if (printState) { Serial.println("timerFSM: ALERT"); }
            break;
        case COUNTUP:
            if (printState) { Serial.println("timerFSM: COUNTUP"); }
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

    #if DEBUG
    debugStatePrint(true);
    #endif

    switch(currentState) {
        case IDLE_ST:
            break;
        case COUNTDOWN:
            break;
        case ALERT:
            break;
        case COUNTUP:
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state update hit default");
            #endif
            break;
    }

    switch(currentState) {
        case IDLE_ST:
            break;
        case COUNTDOWN:
            break;
        case ALERT:
            break;
        case COUNTUP:
            break;
        default:
            #if DEBUG
            Serial.println("timerFSM_tick(): state transition hit default");
            #endif
            break;
    }
}