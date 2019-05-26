#include <Arduino.h>
#include <Encoder.h>

#include "ringTimeCommon.h"
#include "encDelta.h"
#include "buttonFSM.h"
#include "timerFSM.h"
#include <avr/sleep.h>
#include <avr/wdt.h>

#define DEBUG 0

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect

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
  timerFSM_tick(pressed, change);
}

void setup() {
  for (byte i = 0; i <= A5; i++)
    {
    pinMode (i, OUTPUT);    // changed as per below
    digitalWrite (i, LOW);  //     ditto
    }

  #if DEBUG
  Serial.begin(BAUD_RATE);
  #endif
  buttonFSM_init();
  timerFSM_init();
  // Timer1.initialize(TICK_U_SECONDS);
  // #if DEBUG
  // Timer1.attachInterrupt(isr_timerDone); // Testing
  // #else
  // Timer1.attachInterrupt(update); // Production
  // #endif
}

#if DEBUG
void loop() {
  update();
  while(!timerDone);
  timerDone = false;
}
#else
void loop() {
  // disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit(WDCE) | bit(WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit(WDIE) | bit(WDP0);    // set WDIE, and 32ms delay
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();

   update();

  // LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_ON, 
  //               SPI_OFF, USART0_OFF, TWI_OFF);
}
#endif