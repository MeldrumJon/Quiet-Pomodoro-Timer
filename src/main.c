#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include "controller.h"
#include "lcd.h"
#include "btn.h"
#include "encoder.h"
#include "timer.h"

int main(void) {
    // Power Saving
    ADCSRA = 0; // Disable ADC
    power_all_disable();
    power_timer2_enable();

    // Output low
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;

    btn_enable();
    encoder_enable();
    timer_enable();

    //power_usart0_enable();
    //usart_init();
    //stdout = &uart_stream;
    //printf("Hello world!\r\n");

    for (;;) {
        if (btn_intrpted()) {
            controller_wake();
        }

        if (btn_pressed()) {
            controller_press();
        }

        if (btn_longpressed()) {
            controller_longpress();
        }

        int8_t delta = encoder_delta();
        if (delta) {
            controller_rotate(delta);
        }

        if (timer_intrpted()) {
            btn_tick();
            controller_tick();
        }

        // Low power sleep
        if (controller_inDeepSleep()) {
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        }
        else {
            set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        }
        cli();
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
    }
}
