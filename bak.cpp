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

#define TEXT_SIZE 1
#define TEXT_MODE 1

#define SCREEN_WIDTH 130
#define SCREEN_HEIGHT 130
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7

static LCDWIKI_SPI mylcd(SSD1283A, 10, 9, 8, A3); // model,cs,cd,reset,led

#define ORIGIN_X 65
#define ORIGIN_Y 65
#define SEGMENT_LENGTH 50
static uint8_t circ_close[SEGMENT_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 20, 21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34};
static uint8_t circ_far[SEGMENT_LENGTH] = {49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 46, 46, 46, 46, 45, 45, 45, 44, 44, 44, 43, 43, 43, 42, 42, 42, 41, 41, 40, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34};

// Circle runs from 0 to 400
void drawSegment(uint16_t from, uint16_t to) {
	uint16_t start, end;
	start = from % 50;
	if (from < 50) { goto tltop; }
	else if (from < 100) { goto tlbottom; }
	else if (from < 150) { goto bltop; }
	else if (from < 200) { goto blbottom; }
	else if (from < 250) { goto brbottom; }
	else if (from < 300) { goto brtop; }
	else if (from < 350) { goto trbottom; }
	else { goto trtop; }
tltop:
	end = (to < 50) ? to : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // TL TOP
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X-circ_close[i], ORIGIN_Y-circ_far[i]);
	}
	if (to < 50) {return;}
	start = 1;
tlbottom:
	end = (to < 100) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // TL Bottom
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X-circ_far[SEGMENT_LENGTH-1-i], ORIGIN_Y-circ_close[SEGMENT_LENGTH-1-i]);
	}
	if (to < 100) {return;}
	start = 1;
bltop:
	end = (to < 150) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // BL Top
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X-circ_far[i], ORIGIN_Y+circ_close[i]);
	}
	if (to < 150) {return;}
	start = 1;
blbottom:
	end = (to < 200) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // BL Bottom
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X-circ_close[SEGMENT_LENGTH-1-i], ORIGIN_Y+circ_far[SEGMENT_LENGTH-1-i]);
	}
	if (to < 200) {return;}
	start = 1;
brbottom:
	end = (to < 250) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // BR bottom
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X+circ_close[i], ORIGIN_Y+circ_far[i]);
	}
	if (to < 250) {return;}
	start = 1;
brtop:
	end = (to < 300) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // BR top
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X+circ_far[SEGMENT_LENGTH-1-i], ORIGIN_Y+circ_close[SEGMENT_LENGTH-1-i]);
	}
	if (to < 300) {return;}
	start = 1;
trbottom:
	end = (to < 350) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // TR bottom
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X+circ_far[i], ORIGIN_Y-circ_close[i]);
	}
	if (to < 350) {return;}
	start = 1;
trtop:
	end = (to < 400) ? to % 50 : SEGMENT_LENGTH;
	for (uint16_t i = start; i < end; ++i) { // TR top
		mylcd.Draw_Line(ORIGIN_X, ORIGIN_Y, ORIGIN_X+circ_close[SEGMENT_LENGTH-1-i], ORIGIN_Y-circ_far[SEGMENT_LENGTH-1-i]);
	}
	return;
}

void setup()
{
	mylcd.Init_LCD();
	mylcd.Fill_Screen(BLACK);

// 	power_adc_disable();
// 	power_spi_disable();
// 	power_timer0_disable();
// 	power_timer1_disable();
// 	power_timer2_disable();
// 	power_twi_disable();
// 	power_usart0_disable();	

	mylcd.Set_Text_Size(TEXT_SIZE);
	mylcd.Set_Text_Mode(TEXT_MODE);
	mylcd.Set_Text_colour(WHITE);
	mylcd.Set_Draw_color(RED);
	
// 	power_usart0_enable(); // Setup UART
// 	usart_init();
// 	stdout = &uart_stream;
// 	usart_sendChar('\r'); // Does the UART work?
// 	usart_sendChar('\n');
// 	usart_sendString("Hello ");
// 	printf("world!\r\n");
// 	
// 	power_timer2_enable();
// 	//Setup Timer2 to fire every 1ms
// 	TCCR2B = 0x00;        //Disbale Timer2 while we set it up
// 	TCNT2  = 130;         //Reset Timer Count to 130 out of 255
// 	TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
// 	TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
// 	TCCR2A = 0x00;        //Timer2 Control Reg A: Wave Gen Mode normal
// 	TCCR2B = 0x05;        //Timer2 Control Reg B: Timer Prescaler set to 128
}

void loop() 
{
	power_spi_enable();

	mylcd.Fill_Screen(BLACK);
	mylcd.Print_String("0", 62, 5);
	mylcd.Print_String("15", 1, 61);
	mylcd.Print_String("30", 59, 118);
	mylcd.Print_String("45", 118, 61);

	mylcd.Fill_Screen(BLACK);
	drawSegment(0, 400);
	delay(500);

	mylcd.Fill_Screen(BLACK);
	drawSegment(80, 200);
	delay(500);
	
	mylcd.Fill_Screen(BLACK);
	drawSegment(33, 167);
	drawSegment(233, 267);
	delay(500);
}

