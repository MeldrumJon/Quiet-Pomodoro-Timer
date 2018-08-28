#include <Arduino.h>
#include <TimerOne.h>
#include <LowPower.h>
#include <Encoder.h>

#include <ringTimeCommon.h>
#include <encDelta.h>
#include <buttonFSM.h>
#include <timerFSM.h>

#define DEBUG 1

volatile static bool timerDone = false;
void isr_timerDone(void) { timerDone = true; }

void update(void) {
  
  uint8_t pressed = buttonFSM_tick(); // Buttons take priority
  #if DEBUG
  if (pressed != 0) {
    Serial.println(pressed, BIN);
  }
  #endif
  timerFSM_tick(pressed);

  int_fast16_t change = encDelta_getChange();
  static int_fast16_t lastChange = 0;
  if (change != lastChange) {
    Serial.println(change, DEC);
    change = lastChange;
  }

}

void setup() {
  #if DEBUG
  Serial.begin(BAUD_RATE);
  #endif
  buttonFSM_init();
  timerFSM_init();
  Timer1.initialize(TICK_U_SECONDS);
  Timer1.attachInterrupt(isr_timerDone);
}

void loop() {
  update();
  while(!timerDone);
  timerDone = false;
}