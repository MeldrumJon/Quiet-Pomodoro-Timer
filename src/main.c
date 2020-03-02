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

    lcd_init();


    lcd_set_cursor(LCD_LINE2, 4);
    lcd_write_str("Hello world!");

    lcd_put_cgram_same(0, 0x00);
    uint8_t data = 0x10;
    for (uint8_t i = 1; i <= 5; ++i) {
        lcd_put_cgram_same(i, data);
        data |= data >> 1;
    }

    lcd_set_cursor(LCD_LINE1, 0);
    for (uint8_t i = 0; i < 6; ++i) {
        lcd_write_ch(i);
    }

    for (;;) {
        _delay_ms(1000);
        PORTD &= ~(0x60);
        _delay_ms(1000);
        PORTD |= 0x60;
    }
}
