#include <stdint.h>

#define DISP_CONTRAST_ON 0xF
#define DISP_CONTRAST_DIM 0x5

#define DISP_GREEN 5
#define DISP_BLUE 0
#define DISP_RED 11

void disp_init(void);
void disp_setContrast(uint8_t x);
void disp_off(void);
void disp_on(void);
void disp_clear(void);

void disp_drawTime(uint8_t mins, uint8_t color);
void disp_diffTime(uint8_t mins, uint8_t color);
void disp_alert(uint_fast8_t show_clear_n);