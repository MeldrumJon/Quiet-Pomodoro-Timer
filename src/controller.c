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

#define WAKING_ST_TIMEOUT TIMER_TICKS(0.250)
#define SET_ST_TIMEOUT TIMER_TICKS(30)
#define ALERT_ST_TIMEOUT TIMER_TICKS(60)
#define CALIBRATE_ST_TIMEOUT TIMER_TICKS(15)

#define LED_ON_TIME 1
#define LED_CYCLE_TIME TIMER_TICKS(2)

#define MINUTE_LINE LCD_LINE1
#define MINUTE_IDX 11
#define STATE_LINE LCD_LINE1
#define STATE_IDX 15

#define CALIB_LINE0_IDX 1
#define CALIB_LINE1_IDX 0
static const char calib_str0[] = "Press btn then";
static const char calib_str1[] = "Press after 1min";

static uint16_t timeout_cnt;
static uint16_t per_cnt;

static int8_t minutes = DEFAULT_MINUTES;
static int8_t countdown_minutes;

static uint16_t ticks_per_min = TIMER_TICKS(60);

// rotate 360deg three times
#define CALIBRATE_DETENTS (12*3)
static int8_t calibrate_rot;

static char buf[4]; // len(MAX_MINUTES as string) + 1

static enum controller_st_t {
	DEEPSLEEP_ST,
	WAKING_ST,
	SET_ST,
	COUNTDOWN_ST,
	ALERT_ST,
	CALIBRATE_ST,
	CALIBRATING_ST
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
    //    case CALIBRATE_ST:
    //        printf("CALIBRATE_ST\r\n");
    //        break;
    //    case CALIBRATING_ST:
    //        printf("CALIBRATING_ST\r\n");
    //        break;
    //}
    // END DEBUG

    switch (newState) {
        case DEEPSLEEP_ST:
            lcd_off();
            encoder_disable(); // Leaving enabled seemed to cause some current draw
            led_off(); // just in case
            break;
        case WAKING_ST:
            timeout_cnt = 0;
            break;
        case SET_ST:
            if (currentState == WAKING_ST) {
                lcd_on();
                encoder_enable();
                lcd_set_cgram_progress();
                lcd_set_cgram_modes();
            }
            else {
                lcd_clear();
            }

            //printf("Set: %d mins\r\n", minutes);

            lcd_write_progressbar(minutes);
            sprintf(buf, "%3d", minutes);
            lcd_write_str(MINUTE_LINE, MINUTE_IDX, buf);
            lcd_write_ch(STATE_LINE, STATE_IDX, 4);
            timeout_cnt = 0;
            calibrate_rot = 0;
            break;
        case COUNTDOWN_ST:
            countdown_minutes = minutes;
            timeout_cnt = 0;

            //printf("Countdown: %d remaining\r\n", countdown_minutes);

            lcd_write_progressbar(countdown_minutes);
            sprintf(buf, "%3d", countdown_minutes);
            lcd_write_str(MINUTE_LINE, MINUTE_IDX, buf);
            lcd_write_ch(STATE_LINE, STATE_IDX, 5);
            break;
        case ALERT_ST:
            lcd_write_ch(STATE_LINE, STATE_IDX, 6);
            timeout_cnt = 0;
            per_cnt = 0;
            break;
        case CALIBRATE_ST:
            lcd_clear();
            timeout_cnt = 0;
            lcd_write_str(LCD_LINE0, CALIB_LINE0_IDX, calib_str0);
            lcd_write_str(LCD_LINE1, CALIB_LINE1_IDX, calib_str1);
            break;
        case CALIBRATING_ST:
            lcd_clear();
            lcd_write_str(LCD_LINE1, CALIB_LINE1_IDX, calib_str1);
            timeout_cnt = 0;
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
        case CALIBRATE_ST:
            // Do nothing
            break;
        case CALIBRATING_ST:
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
        case CALIBRATE_ST:
            controller_switchState(CALIBRATING_ST);
            break;
        case CALIBRATING_ST:
            ticks_per_min = timeout_cnt;
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
                calibrate_rot += minutes; // goes negative
                minutes = 0;
            }
            else if (minutes > MAX_MINUTES) {
                minutes = MAX_MINUTES;
                calibrate_rot = 0;
            }
            else {
                calibrate_rot = 0;
            }

            if (calibrate_rot <= (-CALIBRATE_DETENTS)) {
                controller_switchState(CALIBRATE_ST);
                break;
            }

            //printf("Set: %d mins\r\n", minutes);

            lcd_write_progressbar(minutes);
            sprintf(buf, "%3d", minutes);
            lcd_write_str(MINUTE_LINE, MINUTE_IDX, buf);
            timeout_cnt = 0;
            break;
        case COUNTDOWN_ST:
            // Do nothing
            break;
        case ALERT_ST:
            // Do nothing
            break;
        case CALIBRATE_ST:
            // Do nothing
            break;
        case CALIBRATING_ST:
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
            if (timeout_cnt >= ticks_per_min) {
                --countdown_minutes;
                //printf("Countdown: %d remaining\r\n", countdown_minutes);

                lcd_write_progressbar(countdown_minutes);
                sprintf(buf, "%3d", countdown_minutes);
                lcd_write_str(MINUTE_LINE, MINUTE_IDX, buf);
                timeout_cnt = 0;
            }
            if (countdown_minutes == 0) {
                controller_switchState(ALERT_ST);
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
        case CALIBRATE_ST:
            ++timeout_cnt;
            if (timeout_cnt >= CALIBRATE_ST_TIMEOUT) {
                controller_switchState(DEEPSLEEP_ST);
            }
            // Do nothing
            break;
        case CALIBRATING_ST:
            ++timeout_cnt;
            break;
 
    }
}
