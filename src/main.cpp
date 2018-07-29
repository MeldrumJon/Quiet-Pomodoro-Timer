#include <Arduino.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <ringTimeCommon.h>
#include <buttonFSM.h>
#include <timerFSM.h>

volatile static bool timerDone = false;
void isr_timerDone(void) { timerDone = true; }

void update(void) {
  uint8_t pressed = buttonFSM_tick();
  if (pressed != 0) {
    Serial.println(pressed, BIN);
  }
  timerFSM_tick(pressed);
}

void setup() {
  Serial.begin(BAUD_RATE);
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