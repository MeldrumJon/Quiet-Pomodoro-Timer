#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "lcd.h"

// Ports and Pins
#define DATA_PORT PORTB
#define CTRL_PORT PORTC

#define CTRL_POWER_MASK 0x1
#define CTRL_E_MASK 0x2
#define CTRL_RW_MASK 0x4
#define CTRL_RS_MASK 0x8

// Wait for more than 30ms after VDD rises to 4.5V
#define POWERON_MS (30+200)

// Commands
#define CMD_CLEARDISPLAY    0x01
#define CMD_CLEARDISPLAY_US (1530+200)

#define CMD_RETURNHOME    0x02
#define CMD_RETURNHOME_US (1530+200)

#define CMD_ENTRYMODESET    0x04
#define CMD_ENTRYMODESET_US (39+20)
#define ENTRYMODESET_ID 0x2
#define ENTRYMODESET_SH 0x1

#define CMD_DISPLAYCONTROL    0x08
#define CMD_DISPLAYCONTROL_US (39+20)
#define DISPLAYCONTROL_D      0x4
#define DISPLAYCONTROL_C      0x2
#define DISPLAYCONTROL_B      0x1

#define CMD_CURSORSHIFT    0x10
#define CMD_CURSORSHIFT_US (39+20)

#define CMD_FUNCTIONSET    0x20
#define CMD_FUNCTIONSET_US (39+20)
#define FUNCTIONSET_DL     0x10
#define FUNCTIONSET_N      0x08
#define FUNCTIONSET_F      0x04

#define CMD_SETCGRAMADDR    0x40
#define CMD_SETCGRAMADDR_US (39+20)

#define CMD_SETDDRAMADDR    0x80
#define CMD_SETDDRAMADDR_US (39+20)

#define WRITE_US (43+20)
#define READ_US (43+20)

// Display Constants
#define CURSOR_OFFSCREEN 0x50
#define BLANK_CHAR 0x20
#define FULL_CHAR 0xFF

#define PROGRESS_CHARS 24
#define PROGRESS_MIN_PER_CHAR 5

#define PROGRESS_1MIN_CG 0x10
#define PROGRESS_2MIN_CG 0x18
#define PROGRESS_3MIN_CG 0x1C
#define PROGRESS_4MIN_CG 0x1E

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8

// Mode indicator
const uint8_t SET_CH[CHAR_HEIGHT] = {
    0b10000,
    0b11111,
    0b10000,
    0b10101,
    0b11111,
    0b10000,
    0b11111,
    0b00001
};

const uint8_t CD_CH[CHAR_HEIGHT] = {
    0b01110,
    0b10001,
    0b10001,
    0b11111,
    0b00000,
    0b10001,
    0b10001,
    0b01110
};

const uint8_t ALT_CH[CHAR_HEIGHT] = {
    0b00000,
    0b00001,
    0b00001,
    0b11111,
    0b00000,
    0b01111,
    0b10100,
    0b01111
};

// RW should always be low
static void lcd_command(uint8_t command) {
    // Assuming RS is already low (should be)
    DATA_PORT = command;

    CTRL_PORT |= CTRL_E_MASK;
    asm volatile("nop"); // hold for at least 195ns
    CTRL_PORT &= ~(CTRL_E_MASK);

    DATA_PORT = 0x00; // This will save a few uA of power
    return;
}

static void lcd_data(uint8_t data) {
    CTRL_PORT |= CTRL_RS_MASK;

    DATA_PORT = data;

    CTRL_PORT |= CTRL_E_MASK;
    asm volatile("nop"); // hold for at least 195ns
    CTRL_PORT &= ~(CTRL_E_MASK);

    // Keep these low--save a few uA of power
    DATA_PORT = 0x00;
    CTRL_PORT &= ~(CTRL_RS_MASK);
    return;
}

static uint8_t cursor = CURSOR_OFFSCREEN;
static uint8_t line0[LCD_LINE_LEN];
static uint8_t line1[LCD_LINE_LEN];

static void lcd_cache_char(uint8_t ddaddr, uint8_t ch) {
    uint8_t offset = ddaddr & LCD_IDX_MASK;
    if (ddaddr & LCD_LINE1) {
        line1[offset] = ch;
    }
    else {
        line0[offset] = ch;
    }
    return;
}

static uint8_t lcd_get_char(uint8_t ddaddr) {
    uint8_t offset = ddaddr & LCD_IDX_MASK;
    if (ddaddr & LCD_LINE1) {
        return line1[offset];
    }
    else {
        return line0[offset];
    }
}

static void lcd_set_cursor(uint8_t ddaddr) {
    if (cursor == ddaddr) {
        return;
    }
    lcd_command(CMD_SETDDRAMADDR | ddaddr);
    _delay_us(CMD_SETDDRAMADDR_US);
    cursor = ddaddr;
    return;
}

static uint8_t lcd_get_ddaddr(uint8_t linemask, uint8_t idx) {
    return (linemask | idx);
}


void lcd_on(void) {
    CTRL_PORT = CTRL_POWER_MASK; // Power LCD
    _delay_ms(POWERON_MS); // Delay more than 30ms after power up

    lcd_command(CMD_FUNCTIONSET | FUNCTIONSET_DL | FUNCTIONSET_N);
    _delay_us(CMD_FUNCTIONSET_US);

    lcd_command(CMD_DISPLAYCONTROL | DISPLAYCONTROL_D);
    _delay_us(CMD_DISPLAYCONTROL_US);

    lcd_clear();

    lcd_command(CMD_ENTRYMODESET | ENTRYMODESET_ID);
    _delay_us(CMD_ENTRYMODESET_US);
    return;
}

void lcd_off(void) {
    DATA_PORT = 0x00;
    CTRL_PORT = 0x00;
    return;
}

void lcd_clear(void) {
    lcd_command(CMD_CLEARDISPLAY);
    _delay_us(CMD_CLEARDISPLAY_US);
    memset(line0, BLANK_CHAR, LCD_LINE_LEN*sizeof(uint8_t));
    memset(line1, BLANK_CHAR, LCD_LINE_LEN*sizeof(uint8_t));
    cursor = 0;
    return;
}

void lcd_write_ch(uint8_t linemask, uint8_t idx, uint8_t ch) {
    uint8_t ddaddr = lcd_get_ddaddr(linemask, idx);
    if (lcd_get_char(ddaddr) != ch) {
        lcd_set_cursor(ddaddr);
        lcd_data(ch);
        _delay_us(WRITE_US);

        lcd_cache_char(ddaddr, ch);
        ++cursor;
    }
    return;
}

void lcd_write_str(uint8_t linemask, uint8_t idx, char* str_ptr) { // no empty strings please
    while (*str_ptr != '\0') {
        lcd_write_ch(linemask, idx, *str_ptr);
        ++idx;
        ++str_ptr;
    }
    return;
}

// chaddr only 0-8, lower 5 bits of data are the only ones which matter
// data should be 8 lines long
// must call lcd_set_cursor afterward!
void lcd_put_cgram(uint8_t chaddr, uint8_t* data) {
    cursor = CURSOR_OFFSCREEN; // current address is cgram, not display
    lcd_command(CMD_SETCGRAMADDR | (chaddr << 3)); // log2(CHAR_HEIGHT)
    _delay_us(CMD_SETCGRAMADDR_US);
    for (uint8_t i = 0; i < CHAR_HEIGHT; ++i) {
        lcd_data(*data);
        _delay_us(WRITE_US);
        ++data;
    }
    return;
}

void lcd_put_cgram_same(uint8_t chaddr, uint8_t data) {
    cursor = CURSOR_OFFSCREEN; // current address is cgram, not display
    lcd_command(CMD_SETCGRAMADDR | (chaddr << 3));
    _delay_us(CMD_SETCGRAMADDR_US);
    for (uint8_t i = 0; i < CHAR_HEIGHT; ++i) {
        lcd_data(data);
        _delay_us(WRITE_US);
    }
    return;
}


void lcd_set_cgram_progress(void) {
    lcd_put_cgram_same(0, PROGRESS_1MIN_CG);
    lcd_put_cgram_same(1, PROGRESS_2MIN_CG);
    lcd_put_cgram_same(2, PROGRESS_3MIN_CG);
    lcd_put_cgram_same(3, PROGRESS_4MIN_CG);
    return;
}

void lcd_write_progressbar(uint8_t count) {
    uint8_t full = count / PROGRESS_MIN_PER_CHAR;
    uint8_t partial = count % PROGRESS_MIN_PER_CHAR;

    for (uint8_t i = 0; i < PROGRESS_CHARS; ++i) {
        uint8_t linemask;
        uint8_t idx;
        if (i >= LCD_LINE_LEN) {
            linemask = LCD_LINE1;
            idx = i - LCD_LINE_LEN;
        }
        else {
            linemask = LCD_LINE0;
            idx = i;
        }
        uint8_t ch = BLANK_CHAR;
        if (i < full) {
            ch = FULL_CHAR;
        }
        else if (i == full && partial > 0) {
            ch = partial - 1;
        }
        else {
            ch = BLANK_CHAR;
        }
        lcd_write_ch(linemask, idx, ch);
    }
    return;
}

void lcd_set_cgram_modes(void) {
    lcd_put_cgram(LCD_SET_CH, SET_CH);
    lcd_put_cgram(LCD_CD_CH, CD_CH);
    lcd_put_cgram(LCD_ALT_CH, ALT_CH);
    return;
}
