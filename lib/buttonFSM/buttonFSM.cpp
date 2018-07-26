#include <buttonFSM.h>
#include <Arduino.h>

#define DEBUG 1

#define BTN0_PIN 4
#define BTN1_PIN 2
#define BTN0_MASK (1<<BTN0_PIN)
#define BTN1_MASK (1<<BTN1_PIN)
#define BTN_MASK (BTN0_MASK|BTN1_MASK)

enum buttonFSM_state_t {
    IDLE_ST,
    WAIT_ST,
    STABLE_ST,
    STABLE_LONG_ST
};
static buttonFSM_state_t currentState = IDLE_ST;

static uint8_t btns = 0;

void buttonFSM_init() {
    DDRD = DDRD & ~BTN_MASK; // set direction for buttons to input
    currentState = IDLE_ST; // should be exactly the same as the static variables at the top
    btns = 0;
    return;
}

// Helper functions
static inline uint8_t readBtns() {
    return PIND & BTN_MASK; // read only the buttons from the digital register
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
            // if (previousState == wait_unpressed_st) { // Is something like this necesary?
                Serial.println(btns, BIN); // Print the state of the buttons after a short press
            // }
            break;
        case STABLE_LONG_ST:
            if (printState) { Serial.println("buttonFSM: STABLE_LONG_ST"); }
            Serial.println(btns, BIN); // Print the state of the buttons after a long press
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

void buttonFSM_tick() {
    static uint16_t counter;
    static uint8_t savedBtns = 0;

    #if DEBUG
    debugStatePrint(true);
    #endif

    switch(currentState) {
        case IDLE_ST:
            break;
        case WAIT_ST:
            break;
        case STABLE_ST:
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
            if (btns != 0) {
                savedBtns = btns;
            }
            break;
        case WAIT_ST:
            break;
        case STABLE_ST:
            break;
        case STABLE_LONG_ST:
            break;
        default:
            #if DEBUG
            Serial.println("buttonFSM_tick(): state transition hit default");
            #endif
            break;
    }

    return;
}