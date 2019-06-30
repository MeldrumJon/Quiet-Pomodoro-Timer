#include <Arduino.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <stdio.h>
#include "usart.h"
#include "timer.h"
#include "encoder.h"
#include "btn.h"
#include "disp.h"
#include "controller.h"

void setup(void) {
	// Power saving
	ADCSRA = 0; // disable ADC
	for (uint8_t i = 0; i <= A5; i++) { // Lower power on pins
		pinMode (i, INPUT);
		digitalWrite (i, LOW);
	}
	power_all_disable();
	power_spi_enable();
	power_timer2_enable();

// 	power_usart0_enable();
// 	usart_init();
// 	stdout = &uart_stream;
// 	printf("Hello world!\r\n");

	timer_enable();
	btn_enable();
	disp_init();
	// Encoder managed in controller.
	controller_tick(); // Get out of the init state.
}

void loop(void) {	
	cli();
	if (btn_intrpt_flag) {
		btn_intrpt_flag = 0;
		sei();
		
		controller_wakeup();
	}
	sei();
	
	cli();
	if (timer_intrpt_flag) {
		timer_intrpt_flag = 0;
		sei();
		
		btn_tick();
		controller_tick();
	}
	sei();
	
	cli();
	if (controller_isOff()) {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	}
	else {
		set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	}
  	sleep_enable();
  	sei();
  	sleep_cpu();
  	sleep_disable();
}
