#include "wdt.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h> // only used for wdt_reset() = wdr assembly instruction
#include "bool.h"

// Whether or not the watchdog timer has expired since the last call to wdt_handleTimout.
// Accesses to this variable should be atomic.
volatile static bool g_timeout = FALSE;

/**
 * Called when the watchdog timer expires.
 * ISR_BLOCK disables interrupts while this ISR executes.
 */
ISR(WDT_vect, ISR_BLOCK) {
	g_timeout = TRUE;
}

/**
 * Returns TRUE if the watchdog timer expired recently.
 *
 * <p>
 * If the watchdog timer expired after the previous call to wdt_handleTimeout (or the
 * beginning of execution), wdt_handleTimout will return TRUE.  Otherwise, it will return
 * FALSE.
 * </p>
 * 
 * @return TRUE if the WDT timed out since the last call, FALSE otherwise.
 */
bool wdt_handleTimeout(void) {
	bool timeout;

	cli(); // Accesses to g_timeout should be atomic
	timeout = g_timeout;
	g_timeout = FALSE;
	sei();

	return timeout;
}

/**
 * Disables the watchdog timer.
 * 
 * <p>
 * This code should be called at the beginning of main().
 * </p>
 */
void wdt_off(void) {

	cli();
	MCUSR &= ~_BV(WDRF); // Clear watchdog reset flag in status register
	WDTCSR |= _BV(WDCE) | _BV(WDE); // Enabled changes to the watchdog timer register
	WDTCSR = 0x00; // Turn off the watchdog timer
	sei();

	return;
}

/**
 * Enable the watchdog timer.
 * 
 * <p>
 * The timer restarts each time it expires. There's no need to call wdt_start every
 * interval.
 * </p>
 *
 * @param wdt_scaler_mask Bits to set in the WDTCSR register to set the expire time.  The
 *                        masks are defined in "wdt.h" (WDT_16MS_MSK to WDT_8S_MSK).
 */
void wdt_start(uint8_t wdt_timeout_msk) {

	cli();
	wdt_reset(); // Becomes "wdr" assembly instruction.
	WDTCSR |= _BV(WDCE) | _BV(WDE); // Enable changes to the watchdog timer register.
	WDTCSR = _BV(WDIE) | wdt_timeout_msk; // Enable interrupts (but not reset), set time.
	sei();
	
	return;
}
