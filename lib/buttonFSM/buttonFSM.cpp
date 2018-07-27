#include <buttonFSM.h>
#include <Arduino.h>
#include <ringTimeCommon.h>

#define DEBUG 0

#define DEBOUNCE_U_SECONDS 60000
#define DEBOUNCE_TICKS (DEBOUNCE_U_SECONDS/TICK_U_SECONDS)
#define HOLD_U_SECONDS 3000000
#define HOLD_TICKS (HOLD_U_SECONDS/TICK_U_SECONDS)

enum buttonFSM_state_t {
    IDLE_ST,
    WAIT_ST,
    STABLE_ST,
    STABLE_LONG_ST
};
static buttonFSM_state_t currentState = IDLE_ST;

void buttonFSM_init() {
    DDRD = DDRD & ~BTNS_MASK; // set direction for buttons to input
    currentState = IDLE_ST; // should be exactly the same as the static variables at the top
    return;
}

// Helper functions
static inline uint8_t readBtns() {
    return PIND & BTNS_MASK; // read only the buttons from the digital register
}

// FSM
# if DEBUG
static void debugStatePrint(bool printState) {
    static buttonFSM_state_t previousState = IDLE_ST;
    static bool firstPass = true;
    if (previousState != currentState || firstPass) { // only print after a state transition
        firstPass = false;
        switch(currentState) {
        case IDLE_ST:
            if (printState) { Serial.println("buttonFSM: IDLE_ST"); }
            break;
        case WAIT_ST:
            if (printState) { Serial.println("buttonFSM: WAIT_ST"); }
            break;
        case STABLE_ST:
            if (printState) { Serial.println("buttonFSM: STABLE_ST"); }
            break;
        case STABLE_LONG_ST:
            if (printState) { Serial.println("buttonFSM: STABLE_LONG_ST"); }
            break;
        default: // error, we forgot to include a state
            Serial.println("buttonFSM debugStatePrint: hit default");
            break;
        }
    }
    previousState = currentState;
    return;
}
#endif

uint8_t buttonFSM_tick() {
    static uint16_t counter = 0;
    static uint8_t prevBtns = 0;
    uint8_t btns;
    uint8_t pressedBtns = 0;

    #if DEBUG
    debugStatePrint(true);
    #endif

    switch(currentState) {
        case IDLE_ST:
            break;
        case WAIT_ST:
            ++counter;
            break;
        case STABLE_ST:
            ++counter;
            break;
        case STABLE_LONG_ST:
            break;
        default:
            #if DEBUG
            Serial.println("buttonFSM_tick(): state update hit default");
            #endif
            break;
    }

    switch(currentState) {
        case IDLE_ST:
            btns = readBtns();
            if (btns != 0) {
                prevBtns = btns;
                counter = 0;
                currentState = WAIT_ST;
            }
            break;
        case WAIT_ST:
            btns = readBtns();
            if (btns != prevBtns) {
                currentState = IDLE_ST;
            }
            else if (counter >= DEBOUNCE_TICKS) {
                pressedBtns = btns;
                counter = 0;
                currentState = STABLE_ST;
            }
            break;
        case STABLE_ST:
            btns = readBtns();
            if (btns != prevBtns) {
                currentState = IDLE_ST;
            }
            else if (counter >= HOLD_TICKS) {
                pressedBtns = LONG_PRESS_ADJUST(btns);
                currentState = STABLE_LONG_ST;
            }
            break;
        case STABLE_LONG_ST:
            btns = readBtns();
            if (btns != prevBtns) {
                currentState = IDLE_ST;
            }
            break;
        default:
            #if DEBUG
            Serial.println("buttonFSM_tick(): state transition hit default");
            #endif
            break;
    }
    return pressedBtns;
}