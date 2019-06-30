#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include "disp.h"

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
#define GREY            0x8410

static int8_t circle_x[] = { -26, -45, -52, -45, -26, 0, 26, 45, 52, 45, 26, 0 };
static int8_t circle_y[] = { -45, -26, 0, 26, 45, 52, 45, 26, 0, -26, -45, -52 };

static char minute_str[] = "   ";
static uint8_t last_mins = 0;
static uint8_t last_circles = 0;
static uint8_t last_fade = 0;

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, 10, 9, 8);

void disp_init(void) {
	tft.begin();
	tft.setTextSize(3);
	tft.setTextWrap(false);
	tft.setTextColor(GREY, BLACK);
}

/*
 * Display sleep mode
 */

// x is between 0x0 and 0xF
void disp_setContrast(uint8_t x) {
	tft.sendCommand(SSD1351_CMD_CONTRASTMASTER, &x, 1);
}

void disp_off(void) {
	tft.sendCommand(SSD1351_CMD_DISPLAYOFF, (const uint8_t *) NULL, 0);
	uint8_t cmd = 0x0;
	tft.sendCommand(SSD1351_CMD_FUNCTIONSELECT, &cmd, 1);
}

void disp_on(void) {
	uint8_t cmd = 0x1;
	tft.sendCommand(SSD1351_CMD_FUNCTIONSELECT, &cmd, 1);
	_delay_ms(1);
	tft.sendCommand(SSD1351_CMD_DISPLAYON, (const uint8_t *) NULL, 0);
}

void disp_clear(void) {
	tft.fillScreen(BLACK);
	return;
}

/*
 * Display Drawing
 */

void disp_drawTime(uint8_t mins, uint8_t color) {
	uint8_t circles = mins / 5;
	uint16_t circle_color = (color == DISP_BLUE) ? BLUE : (color == DISP_GREEN) ? GREEN : RED;
	uint8_t fade = mins % 5;
	uint16_t fade_color = (6*fade) << color;
	sprintf(minute_str, "%02d", mins);
	
	tft.fillScreen(BLACK);
	
	tft.setCursor(SCREEN_CENTER_X-(TEXT_WIDTH-TEXT_SCALE/2), SCREEN_CENTER_Y-(TEXT_HEIGHT-TEXT_SCALE)/2); // Center 2 chars on screen
	tft.print(minute_str);
	
	uint8_t i;
	for (i = 0; i < circles; ++i) {
		tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, circle_color);
	}
	
	tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, fade_color);
	
	last_mins = mins;
	last_circles = circles;
	last_fade = fade;
	
	return;
}

void disp_redrawCircles(uint8_t mins, uint8_t color) {
	uint8_t circles = mins / 5;
	uint16_t circle_color = (color == DISP_BLUE) ? BLUE : (color == DISP_GREEN) ? GREEN : RED;
	uint8_t fade = mins % 5;
	uint16_t fade_color = (6*fade) << color;
	
	uint8_t i;
	for (i = 0; i < circles; ++i) {
		tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, circle_color);
	}
	
	tft.fillCircle(SCREEN_CENTER_X+circle_x[i], SCREEN_CENTER_Y+circle_y[i], 5, fade_color);
	
	last_mins = mins;
	last_circles = circles;
	last_fade = fade;
}

void disp_diffTime(uint8_t mins, uint8_t color) {
	uint8_t circles = mins / 5;
	uint16_t circle_color = (color == DISP_BLUE) ? BLUE : (color == DISP_GREEN) ? GREEN : RED;
	uint8_t fade = mins % 5;
	uint16_t fade_color = (6*fade) << color;
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
			tft.fillCircle(SCREEN_CENTER_X+circle_x[last_circles], SCREEN_CENTER_Y+circle_y[last_circles], 5, circle_color);
			last_circles++;
		}
		tft.fillCircle(SCREEN_CENTER_X+circle_x[circles], SCREEN_CENTER_Y+circle_y[circles], 5, fade_color);
	}
	
	last_mins = mins;
	last_circles = circles;
	last_fade = fade;
	
	return;
}

void disp_alert(uint_fast8_t show_clear_n) {
	if (show_clear_n) {
		tft.setCursor(SCREEN_CENTER_X-(TEXT_WIDTH*3-TEXT_SCALE/2), SCREEN_CENTER_Y-(TEXT_HEIGHT-TEXT_SCALE/2)); // Center 2 chars on screen
		tft.print("Time's");
		tft.setCursor(SCREEN_CENTER_X-(TEXT_WIDTH*1.5-TEXT_SCALE/2), SCREEN_CENTER_Y); // Center 2 chars on screen
		tft.print("up!");
	}
	else {
		tft.fillScreen(BLACK);
	}
}