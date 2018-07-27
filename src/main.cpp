#include <Arduino.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <TimerOne.h>
#include <LowPower.h>
#include <buttonFSM.h>
#include <ringTimeCommon.h>

#define DATA_PIN 6
#define NUM_LEDS 12
CRGB leds[NUM_LEDS];

#define NUM_COLORS 3
const CRGB COLORS[NUM_COLORS] = {0x110000, 0x001100, 0x000011};

volatile static bool timerDone = false;
void isr_timerDone(void) { timerDone = true; }

void update(void) {
  volatile static int led_index = 0;
  volatile static int color_index = 0;
  leds[led_index] = COLORS[color_index];
  led_index = (led_index + 1) % NUM_LEDS;
  if (led_index == 0) {
    color_index = (color_index + 1) % NUM_COLORS;
  }
  FastLED.show();
  uint8_t pressed = buttonFSM_tick();
  if (pressed != 0) {
    Serial.println(pressed, BIN);
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  buttonFSM_init();
  FastLED.addLeds<WS2812, DATA_PIN>(leds, NUM_LEDS);
  Timer1.initialize(TICK_U_SECONDS);
  Timer1.attachInterrupt(isr_timerDone);
}

void loop() {
  update();
  while(!timerDone);
  timerDone = false;
}