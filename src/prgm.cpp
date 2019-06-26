/*************************************************** 
  This is a example sketch demonstrating graphic drawing
  capabilities of the SSD1351 library for the 1.5" 
  and 1.27" 16-bit Color OLEDs with SSD1351 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673
 
  If you're using a 1.27" OLED, change SCREEN_HEIGHT to 96 instead of 128.

  These displays use SPI to communicate, 4 or 5 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution

  The Adafruit GFX Graphics core library is also required
  https://github.com/adafruit/Adafruit-GFX-Library
  Be sure to install it!
 ****************************************************/

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)

#define TEXT_SCALE 3
#define TEXT_WIDTH (6*TEXT_SCALE)
#define TEXT_HEIGHT (8*TEXT_SCALE)

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF
#define GREY 0x8410

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <avr/power.h>

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, 10, 9, 8);

/*
 * Display sleep mode
 */

void setContrast(uint8_t x) {
	tft.sendCommand(SSD1351_CMD_CONTRASTMASTER, &x, 1);
}

void displayOff() {
	tft.sendCommand(SSD1351_CMD_DISPLAYOFF, (const uint8_t *) NULL, 0);
	uint8_t cmd = 0x0;
	tft.sendCommand(SSD1351_CMD_FUNCTIONSELECT, &cmd, 1);
}

void displayOn() {
	uint8_t cmd = 0x1;
	tft.sendCommand(SSD1351_CMD_FUNCTIONSELECT, &cmd, 1);
	_delay_ms(1);
	tft.sendCommand(SSD1351_CMD_DISPLAYON, (const uint8_t *) NULL, 0);
}

/*
 * Display Drawing
 */

static int8_t circle_x[] = { -26, -45, -52, -45, -26, 0, 26, 45, 52, 45, 26, 0 };
static int8_t circle_y[] = { -45, -26, 0, 26, 45, 52, 45, 26, 0, -26, -45, -52 };

static char minute_str[] = "   ";
static uint8_t last_mins = 0;
static uint8_t last_circles = 0;
static uint8_t last_fade = 0;

void drawTime(uint8_t mins) {
	uint8_t circles = mins / 5;
	uint8_t fade = mins % 5;
	uint16_t fade_color = (6*fade) << 11;
	sprintf(minute_str, "%02d", mins);
	
	tft.fillScreen(BLACK);
	
	tft.setCursor(SCREEN_CENTER_X-(TEXT_WIDTH-TEXT_SCALE/2), SCREEN_CENTER_Y-(TEXT_HEIGHT-TEXT_SCALE)/2); // Center 2 chars on screen
	tft.print(minute_str);
	
	uint8_t i;
	for (i = 0; i < circles; ++i) {
		tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, RED);
	}
	
	tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, fade_color);
	
	last_mins = mins;
	last_circles = circles;
	last_fade = fade;
	
	return;
}

void diffTime(uint8_t mins) {
	uint8_t circles = mins / 5;
	uint8_t fade = mins % 5;
	uint16_t fade_color = (6*fade) << 11;
	sprintf(minute_str, "%02d", mins);
	
	tft.setCursor(SCREEN_CENTER_X-(TEXT_WIDTH-TEXT_SCALE/2), SCREEN_CENTER_Y-(TEXT_HEIGHT-TEXT_SCALE)/2); // Center 2 chars on screen
	tft.print(minute_str);
	
	if (mins < last_mins) {
		while (last_circles > circles) {
			tft.fillCircle(SCREEN_CENTER_X+circle_x[last_circles], SCREEN_CENTER_Y+circle_y[last_circles], 5, BLACK);
			last_circles--;
		}
		tft.fillCircle(SCREEN_CENTER_X+circle_x[circles], SCREEN_CENTER_Y+circle_y[circles], 5, fade_color);
	}
	else {
		while (last_circles < circles) {
			tft.fillCircle(SCREEN_CENTER_X+circle_x[last_circles], SCREEN_CENTER_Y+circle_y[last_circles], 5, RED);
			last_circles++;
		}
		tft.fillCircle(SCREEN_CENTER_X+circle_x[circles], SCREEN_CENTER_Y+circle_y[circles], 5, fade_color);
	}
	
	last_mins = mins;
	last_circles = circles;
	last_fade = fade;
	
	return;
}

/*
 * Main
 */

void setup(void) {
	// Power saving
	ADCSRA = 0; // disable ADC
	for (uint8_t i = 0; i <= A5; i++) { // Lower power on pins
		pinMode (i, INPUT);
		digitalWrite (i, LOW);
	}
	power_adc_disable();
// 	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
// 	power_timer2_disable();
	power_twi_disable();
	power_usart0_disable();	
	// TFT Setup
	tft.begin();
	tft.setTextSize(3);
	tft.setTextWrap(false);
	tft.setTextColor(GREY, BLACK);
	drawTime(0);
}

void loop() {
	for (uint8_t i = 1; i < 60; ++i) {
		diffTime(i);
		_delay_ms(500);
	}
	drawTime(0);
}
