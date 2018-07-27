#define BAUD_RATE 9600

#define TICK_U_SECONDS 20000

// Buttons masks from the register
#define BTN_START_PIN 4
#define BTN_ADD_PIN 2
#define BTN_START_MASK (1<<BTN_START_PIN)
#define BTN_ADD_MASK (1<<BTN_ADD_PIN)
#define BTNS_MASK (BTN_START_MASK|BTN_ADD_MASK)
// Create masks for long presses
// This assumes a gap between each button. If that is not the case,
// Adjust the function so that the masks do not overlap.
#define LONG_PRESS_ADJUST(mask) (mask<<1)
#define LONG_PRESS_START_MASK BTN_MASK_ADJUST(BTN_START_MASK)
#define LONG_PRESS_ADD_MASK BTN_MASK_ADJUST(BTN_ADD_MASK)
#define LONG_PRESS_MASK BTN_MASK_ADJUST(BTNS_MASK)
#define SHORT_LONG_MASK (LONG_BTNS_MASK|BTNS_MASK)