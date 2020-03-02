#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include "lcd.h"

int main(void) {
    // Power Saving
    ADCSRA = 0; // Disable ADC
    power_all_disable();

    // Output low
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;

    for (;;) {
        _delay_ms(1000);
        PORTD &= ~(0x60);
        _delay_ms(1000);
        PORTD |= 0x60;

        _delay_ms(1000);
        lcd_init();
    }
}
