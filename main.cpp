#include <Arduino.h>
#include <TimerOne.h>
#include <LowPower.h>
#include <Encoder.h>

#include "ringTimeCommon.h"
#include "encDelta.h"
#include "buttonFSM.h"
#include "timerFSM.h"

#define DEBUG 1

volatile static bool timerDone = false;
void isr_timerDone(void) { timerDone = true; }

void update(void) {
  int_fast16_t change = encDelta_getChange();
  uint_fast8_t pressed = buttonFSM_tick(); // Buttons take priority
  #if DEBUG
  if (pressed != 0) {
    Serial.print("Btns: ");
    Serial.println(pressed, BIN);
  }
  if (change != 0) {
    Serial.print("Change: ");
    Serial.println(change, DEC);
  }
  #endif
  (*timerFSM_tick)(pressed, change);
}

void setup() {
  #if DEBUG
  Serial.begin(BAUD_RATE);
  #endif
  buttonFSM_init();
  timerFSM_init();
  Timer1.initialize(TICK_U_SECONDS);
  #if DEBUG
  Timer1.attachInterrupt(isr_timerDone); // Testing
  #else
  Timer1.attachInterrupt(update); // Production
  #endif
}

#if DEBUG
void loop() {
  update();
  while(!timerDone);
  timerDone = false;
}
#else
void loop() {
  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_ON, 
                SPI_OFF, USART0_OFF, TWI_OFF);
}
#endif