#include "controller.h"
#include <stdio.h>
#include <stdbool.h>
#include "btn.h"
#include "encoder.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"

#define DEFAULT_MINUTES 25
#define MAX_MINUTES 120

#define WAKING_ST_TIMEOUT TIMER_TICKS(0.25)
#define SET_ST_TIMEOUT TIMER_TICKS(15)
#define ALERT_ST_TIMEOUT TIMER_TICKS(60)

#define MINUTE_TIME TIMER_TICKS(60)
//#define MINUTE_TIME TIMER_TICKS(1)

#define LED_ON_TIME 1
#define LED_CYCLE_TIME TIMER_TICKS(2)

#define MINUTE_LINE LCD_LINE2
#define MINUTE_IDX 11

static uint16_t timeout_cnt;
static uint16_t per_cnt;

static int8_t minutes = DEFAULT_MINUTES;
static int8_t countdown_minutes;

static char buf[4]; // len(MAX_MINUTES as string) + 1

static enum controller_st_t {
	DEEPSLEEP_ST,
	WAKING_ST,
	SET_ST,
	COUNTDOWN_ST,
	ALERT_ST
} currentState = DEEPSLEEP_ST;

bool controller_inDeepSleep(void) {
    return (currentState == DEEPSLEEP_ST);
}

static void controller_switchState(enum controller_st_t newState) {
    // DEBUG
    //switch (newState) {
    //    case DEEPSLEEP_ST:
    //        printf("DEEPSLEEP_ST\r\n");
    //        break;
    //    case WAKING_ST:
    //        printf("WAKING_ST\r\n");
    //        break;
    //    case SET_ST:
    //        printf("SET_ST\r\n");
    //        break;
    //    case COUNTDOWN_ST:
    //        printf("COUNTDOWN_ST\r\n");
    //        break;
    //    case ALERT_ST:
    //        printf("ALERT_ST\r\n");
    //        break;
    //}
    // END DEBUG

    switch (newState) {
        case DEEPSLEEP_ST:
            lcd_off();
            break;
        case WAKING_ST:
            timeout_cnt = 0;
            break;
        case SET_ST:
            if (currentState == WAKING_ST) {
                lcd_on();
                lcd_set_cgram_progress();
                lcd_set_cgram_modes();
            }
            else {
                lcd_clear();
            }

            //printf("Set: %d mins\r\n", minutes);

            lcd_write_progressbar(minutes);
            sprintf(buf, "%3d", minutes);
            lcd_set_cursor(MINUTE_LINE, MINUTE_IDX);
            lcd_write_str(buf);
            lcd_set_cursor(LCD_LINE2, 15);
            lcd_write_ch(4);
            timeout_cnt = 0;
            break;
        case COUNTDOWN_ST:
            countdown_minutes = minutes;
            timeout_cnt = 0;

            //printf("Countdown: %d remaining\r\n", countdown_minutes);

            lcd_write_progressbar(countdown_minutes);
            sprintf(buf, "%3d", countdown_minutes);
            lcd_set_cursor(MINUTE_LINE, MINUTE_IDX);
            lcd_write_str(buf);
            lcd_set_cursor(LCD_LINE2, 15);
            lcd_write_ch(5);
            break;
        case ALERT_ST:
            lcd_set_cursor(LCD_LINE2, 15);
            lcd_write_ch(6);
            timeout_cnt = 0;
            per_cnt = 0;
            break;
    }
    currentState = newState;
}

// Assumes DEEPSLEEP_ST
void controller_wake(void) {
    switch (currentState) {
        case DEEPSLEEP_ST:
            controller_switchState(WAKING_ST);
            break;
        case WAKING_ST:
            // Do nothing
            break;
        case SET_ST:
            // Do nothing
            break;
        case COUNTDOWN_ST:
            // Do nothing
            break;
        case ALERT_ST:
            // Do nothing
            break;
    }
}

void controller_press(void) {
    switch (currentState) {
        case DEEPSLEEP_ST:
            // Do nothing
            break;
        case WAKING_ST:
            controller_switchState(SET_ST);
            break;
        case SET_ST:
            controller_switchState(COUNTDOWN_ST);
            break;
        case COUNTDOWN_ST:
            // Do nothing
            break;
        case ALERT_ST:
            led_off();
            controller_switchState(SET_ST);
            break;
    }
}

void controller_longpress(void) {
    controller_switchState(DEEPSLEEP_ST);
}

void controller_rotate(int8_t delta) {
    switch (currentState) {
        case DEEPSLEEP_ST:
            // Do nothing
            break;
        case WAKING_ST:
            // Do nothing
            break;
        case SET_ST:
            minutes += delta;
            if (minutes < 0) {
                minutes = 0;
            }
            else if (minutes > MAX_MINUTES) {
                minutes = MAX_MINUTES;
            }

            //printf("Set: %d mins\r\n", minutes);

            lcd_write_progressbar(minutes);
            sprintf(buf, "%3d", minutes);
            lcd_set_cursor(MINUTE_LINE, MINUTE_IDX);
            lcd_write_str(buf);
            timeout_cnt = 0;
            break;
        case COUNTDOWN_ST:
            // Do nothing
            break;
        case ALERT_ST:
            // Do nothing
            break;
    }

}

void controller_tick(void) {
    switch (currentState) {
        case DEEPSLEEP_ST:
            // Do nothing
            break;
        case WAKING_ST:
            ++timeout_cnt;
            if (timeout_cnt >= WAKING_ST_TIMEOUT) {
                controller_switchState(DEEPSLEEP_ST);
            }
            break;
        case SET_ST:
            ++timeout_cnt;
            if (timeout_cnt >= SET_ST_TIMEOUT) {
                controller_switchState(DEEPSLEEP_ST);
            }
            break;
        case COUNTDOWN_ST:
            ++timeout_cnt;
            if (timeout_cnt >= MINUTE_TIME) {
                --countdown_minutes;
                //printf("Countdown: %d remaining\r\n", countdown_minutes);

                lcd_write_progressbar(countdown_minutes);
                sprintf(buf, "%3d", countdown_minutes);
                lcd_set_cursor(MINUTE_LINE, MINUTE_IDX);

                lcd_write_str(buf);
                timeout_cnt = 0;

                if (countdown_minutes == 0) {
                    controller_switchState(ALERT_ST);
                }
            }
            // Do nothing
            break;
        case ALERT_ST:
            if (per_cnt == 0) {
                led_on();
                //printf("ALERT_ON\r\n");
            }
            else if (per_cnt == LED_ON_TIME) {
                led_off();
                //printf("ALERT_OFF\r\n");
            }
            ++per_cnt;
            if (per_cnt >= LED_CYCLE_TIME) {
                per_cnt = 0;
            }

            ++timeout_cnt;
            if (timeout_cnt >= ALERT_ST_TIMEOUT) {
                led_off();
                controller_switchState(SET_ST);
            }
            break;
    }
}
