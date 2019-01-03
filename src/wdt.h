#include <stdint.h>

#define WDT_16MS (0)
#define WDT_32MS ((0x1<<WDP0))
#define WDT_64MS ((0x1<<WDP1))
#define WDT_125MS ((0x1<<WDP1) | (0x1<<WDP0))
#define WDT_250MS ((0x1<<WDP2))
#define WDT_500MS ((0x1<<WDP2) | (0x1<<WDP0))
#define WDT_1S ((0x1<<WDP2) | (0x1<<WDP1))
#define WDT_2S ((0x1<<WDP2) | (0x1<<WDP1) | (0x1<<WDP0))
#define WDT_4S ((0x1<<WDP3))
#define WDT_8S ((0x1<<WDP3) | (0x1<<WDP0))

void wdt_off(void);

void wdt_start(uint8_t wdt_time);

uint8_t wdt_handleTimeout(void);
