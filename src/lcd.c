#include <avr/io.h>
#include <util/delay.h>

// Ports and Pins
#define DATA_PORT PORTB
#define CTRL_PORT PORTC

#define CTRL_POWER_MASK 0x1
#define CTRL_E_MASK 0x2
#define CTRL_RW_MASK 0x4
#define CTRL_RS_MASK 0x8

// Power

// Wait for more than 30ms after VDD rises to 4.5V
#define POWERON_MS (30+100)

// Commands
#define CMD_CLEARDISPLAY    0x01
#define CMD_CLEARDISPLAY_US (1530+1000)

#define CMD_RETURNHOME    0x02
#define CMD_RETURNHOME_US (1530+1000)

#define CMD_ENTRYMODESET    0x04
#define CMD_ENTRYMODESET_US (39+100)
#define ENTRYMODESET_ID 0x2
#define ENTRYMODESET_SH 0x1

#define CMD_DISPLAYCONTROL    0x08
#define CMD_DISPLAYCONTROL_US (39+100)
#define DISPLAYCONTROL_D      0x4
#define DISPLAYCONTROL_C      0x2
#define DISPLAYCONTROL_B      0x1

#define CMD_CURSORSHIFT    0x10
#define CMD_CURSORSHIFT_US (39+100)

#define CMD_FUNCTIONSET    0x20
#define CMD_FUNCTIONSET_US (39+100)
#define FUNCTIONSET_DL     0x10
#define FUNCTIONSET_N      0x08
#define FUNCTIONSET_F      0x04

#define CMD_SETCGRAMADDR    0x40
#define CMD_SETCGRAMADDR_US (39+100)

#define CMD_SETDDRAMADDR    0x80
#define CMD_SETDDRAMADDR_US (39+100)

#define VAL_FUNC_SET 0x2

static void lcd_command(uint8_t command) {
    CTRL_PORT &= ~(CTRL_RS_MASK | CTRL_RW_MASK);

    DATA_PORT = command;

    CTRL_PORT |= CTRL_E_MASK;
    asm volatile("nop");
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
