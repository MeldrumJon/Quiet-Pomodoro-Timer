#ifndef LCD_H
#define LCD_H

#define LCD_LINE_LEN 16
#define LCD_LINE0 0x00
#define LCD_LINE1 0x40
#define LCD_IDX_MASK 0x0F

#define LCD_SET_CH 4
#define LCD_CD_CH 5
#define LCD_ALT_CH 6

void lcd_on(void);

void lcd_off(void);

void lcd_clear(void);


void lcd_write_ch(uint8_t linemask, uint8_t idx, uint8_t ch);

void lcd_write_str(uint8_t linemask, uint8_t idx, char* str_ptr); // no empty strings please

void lcd_put_cgram(uint8_t chaddr, uint8_t* data);

void lcd_put_cgram_same(uint8_t chaddr, uint8_t data);

void lcd_set_cgram_progress(void);

void lcd_write_progressbar(uint8_t count);

void lcd_set_cgram_modes(void);

#endif
