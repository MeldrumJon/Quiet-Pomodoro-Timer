#include <avr/io.h>
#include <util/delay.h>

// Ports and Pins
#define DATA_PORT PORTB
#define CTRL_PORT PORTC

#define CTRL_POWER_MASK 0x1
#define CTRL_E_MASK 0x2
#define CTRL_RW_MASK 0x4
#define CTRL_RS_MASK 0x8

// Wait for more than 30ms after VDD rises to 4.5V
#define POWERON_MS (30+100)

// Commands
#define CMD_CLEARDISPLAY    0x01
#define CMD_CLEARDISPLAY_US (1530+100)

#define CMD_RETURNHOME    0x02
#define CMD_RETURNHOME_US (1530+100)

#define CMD_ENTRYMODESET    0x04
#define CMD_ENTRYMODESET_US (39+10)
#define ENTRYMODESET_ID 0x2
#define ENTRYMODESET_SH 0x1

#define CMD_DISPLAYCONTROL    0x08
#define CMD_DISPLAYCONTROL_US (39+10)
#define DISPLAYCONTROL_D      0x4
#define DISPLAYCONTROL_C      0x2
#define DISPLAYCONTROL_B      0x1

#define CMD_CURSORSHIFT    0x10
#define CMD_CURSORSHIFT_US (39+10)

#define CMD_FUNCTIONSET    0x20
#define CMD_FUNCTIONSET_US (39+10)
#define FUNCTIONSET_DL     0x10
#define FUNCTIONSET_N      0x08
#define FUNCTIONSET_F      0x04

#define CMD_SETCGRAMADDR    0x40
#define CMD_SETCGRAMADDR_US (39+10)

#define CMD_SETDDRAMADDR    0x80
#define CMD_SETDDRAMADDR_US (39+10)

#define WRITE_US (43+10)
#define READ_US (43+10)

static void lcd_command(uint8_t command) {
    CTRL_PORT &= ~(CTRL_RS_MASK | CTRL_RW_MASK);

    DATA_PORT = command;

    CTRL_PORT |= CTRL_E_MASK;
    asm volatile("nop"); // hold for at least 195ns
    CTRL_PORT &= ~(CTRL_E_MASK);
}

static void lcd_data(uint8_t data) {
    CTRL_PORT &= ~(CTRL_RW_MASK); // TODO: Do we need this? It's always zero
    CTRL_PORT |= CTRL_RS_MASK;

    DATA_PORT = data;

    CTRL_PORT |= CTRL_E_MASK;
    asm volatile("nop"); // hold for at least 195ns
    CTRL_PORT &= ~(CTRL_E_MASK);
}

void lcd_init(void) {
    CTRL_PORT = CTRL_POWER_MASK; // Power LCD
    _delay_ms(POWERON_MS); // Delay more than 30ms after power up

    lcd_command(CMD_FUNCTIONSET | FUNCTIONSET_DL | FUNCTIONSET_N);
    _delay_us(CMD_FUNCTIONSET_US);

    lcd_command(CMD_DISPLAYCONTROL | DISPLAYCONTROL_D | DISPLAYCONTROL_C | DISPLAYCONTROL_B);
    _delay_us(CMD_DISPLAYCONTROL_US);

    lcd_command(CMD_CLEARDISPLAY);
    _delay_us(CMD_CLEARDISPLAY_US);

    lcd_command(CMD_ENTRYMODESET | ENTRYMODESET_ID);
    _delay_us(CMD_ENTRYMODESET_US);
}

void lcd_off(void) {
    DATA_PORT = 0x00;
    CTRL_PORT = 0x00;
}

void lcd_set_cursor(uint8_t linemask, uint8_t idx) {
    uint8_t ddaddr = idx | linemask;
    lcd_command(CMD_SETDDRAMADDR | ddaddr);
    _delay_us(CMD_SETDDRAMADDR_US);
}

void lcd_write_ch(uint8_t ch) {
    lcd_data(ch);
    _delay_us(WRITE_US);
}

void lcd_write_str(char* str_ptr) { // no empty strings please
    do {
        lcd_write_ch(*str_ptr);
        ++str_ptr;
    } while(*str_ptr != '\0');
}

// chaddr only 0-8, lower 5 bits of data are the only ones which matter
// data should be 8 lines long
// must call lcd_set_cursor afterward!
void lcd_put_cgram(uint8_t chaddr, uint8_t* data) {
    lcd_command(CMD_SETCGRAMADDR | (chaddr << 3));
    _delay_us(CMD_SETCGRAMADDR_US);
    for (uint8_t i = 0; i < 8; ++i) {
        lcd_data(*data);
        _delay_us(WRITE_US);
        ++data;
    }
}

void lcd_put_cgram_same(uint8_t chaddr, uint8_t data) {
    lcd_command(CMD_SETCGRAMADDR | (chaddr << 3));
    _delay_us(CMD_SETCGRAMADDR_US);
    for (uint8_t i = 0; i < 8; ++i) {
        lcd_data(data);
        _delay_us(WRITE_US);
    }
}
