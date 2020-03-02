#define LCD_LINE1 0x00
#define LCD_LINE2 0x40

void lcd_init(void);

void lcd_off(void);

void lcd_set_cursor(uint8_t linemask, uint8_t idx);

void lcd_write_ch(uint8_t ch);

void lcd_write_str(char* str_ptr); // no empty strings please

void lcd_put_cgram(uint8_t chaddr, uint8_t* data);

void lcd_put_cgram_same(uint8_t chaddr, uint8_t data);

