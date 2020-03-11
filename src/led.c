#include <avr/io.h>

#define LED_PORT PORTD
#define LED_MASK 0x60

void led_on(void) {
    LED_PORT |= LED_MASK;
}

void led_off(void) {
    LED_PORT &= ~(LED_MASK);
}
