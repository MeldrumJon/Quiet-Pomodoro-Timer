#define DEBUG

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include "usart.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

LCDWIKI_SPI mylcd(SSD1283A,10,9,8,A3); //hardware spi,cs,cd,reset

extern const unsigned short img[4096] PROGMEM;

unsigned int toggle = 0;  //used to keep the state of the LED
unsigned int count = 0;   //used to keep count of how many interrupts were fired

//Timer2 Overflow Interrupt Vector, called every 1ms
ISR(TIMER2_OVF_vect) {
  count++;               //Increments the interrupt counter
  if(count > 999){
    toggle = !toggle;    //toggles the LED state
    count = 0;           //Resets the interrupt counter
    printf("Timer expired\r\n");
  }
  TCNT2 = 130;           //Reset Timer to 130 out of 255
  TIFR2 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
}; 

void setup()
{
	mylcd.Init_LCD();
	mylcd.Fill_Screen(BLACK);
	
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();
	power_usart0_disable();	
	
	power_usart0_enable(); // Setup UART
	usart_init();
	stdout = &uart_stream;
	usart_sendChar('\r'); // Does the UART work?
	usart_sendChar('\n');
	usart_sendString("Hello ");
	printf("world!\r\n");
	
	power_timer2_enable();
	//Setup Timer2 to fire every 1ms
	TCCR2B = 0x00;        //Disbale Timer2 while we set it up
	TCNT2  = 130;         //Reset Timer Count to 130 out of 255
	TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR2A = 0x00;        //Timer2 Control Reg A: Wave Gen Mode normal
	TCCR2B = 0x05;        //Timer2 Control Reg B: Timer Prescaler set to 128
}

void loop() 
{
	power_spi_enable();

	for (uint16_t i = 0; i < 64; ++i) {
		for (uint16_t j = 0; j < 64; ++j) {
			uint16_t color = pgm_read_word_near(img + (j<<6) + i);
			mylcd.Draw_Pixe(33+i, 33+j, color);
		}
	}
	
	power_spi_disable();
	
	delay(10000);
}

