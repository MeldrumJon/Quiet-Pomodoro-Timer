#include <Arduino.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <stdio.h>
#include "usart.h"
#include "timer.h"
#include "encoder.h"
#include "btn.h"
#include "flags.h"

void setup(void) {
	// Power saving
	ADCSRA = 0; // disable ADC
	for (uint8_t i = 0; i <= A5; i++) { // Lower power on pins
		pinMode (i, OUTPUT);
		digitalWrite (i, LOW);
	}
	power_adc_disable();
// 	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
// 	power_timer2_disable();
	power_twi_disable();
// 	power_usart0_disable();	

	usart_init();
	stdout = &uart_stream;
	printf("Hello world!\r\n");

	timer_enable();
	encoder_enable();
	btn_enable();
}

void loop(void) {
	static uint16_t counter = 0;
	
	cli();
	if (btn_intrpt_flag) {
		btn_intrpt_flag = 0;
		sei();
		
		// Do something with the button interrupt
	}
	sei();
	
	cli();
	if (timer_intrpt_flag) {
		timer_intrpt_flag = 0;
		sei();
		
		btn_tick();
		++counter;
		if (counter >= TIMER_TICKS(4)) {
			counter = 0;
			printf("4s\r\n");
		}
		if (btn_press_flag) {
			int_fast8_t value = encoder_delta();
			printf("%d\r\n", value);
		}
		if (btn_longpress_flag) {
			printf("Button longpress\r\n");
		}
	}
	sei();
	
// 	set_sleep_mode(SLEEP_MODE_PWR_SAVE);  
//   	sleep_enable();
//   	sleep_cpu();
}
