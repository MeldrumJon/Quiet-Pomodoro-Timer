#include <stdint.h>
#include <avr/io.h>
#include "bool.h"

// Watchdog timer values for wdt_start function's wdt_timeout_msk parameter
#define WDT_16MS_MSK  ( 0 )
#define WDT_32MS_MSK  ( _BV(WDP0) )
#define WDT_64MS_MSK  ( _BV(WDP1) )
#define WDT_125MS_MSK ( _BV(WDP1) | _BV(WDP0) )
#define WDT_250MS_MSK ( _BV(WDP2) )
#define WDT_500MS_MSK ( _BV(WDP2) | _BV(WDP0) )
#define WDT_1S_MSK    ( _BV(WDP2) | _BV(WDP1) )
#define WDT_2S_MSK    ( _BV(WDP2) | _BV(WDP1) | _BV(WDP0) )
#define WDT_4S_MSK    ( _BV(WDP3) )
#define WDT_8S_MSK    ( _BV(WDP3) | _BV(WDP0) )

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
bool wdt_handleTimeout(void);

/**
 * Disables the watchdog timer.
 * 
 * <p>
 * This code should be called at the beginning of main().
 * </p>
 */
void wdt_off(void);

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
void wdt_start(uint8_t wdt_timeout_msk);
