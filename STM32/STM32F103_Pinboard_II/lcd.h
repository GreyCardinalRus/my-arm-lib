#ifndef LCD_H
#define LCD_H


extern void lcd_init(void);
extern void setcursor(uint8_t r,uint8_t c);
extern void wr_lcd(uint8_t chr);
extern void wr_lcd_str(uint8_t r,uint8_t c,char *str);
extern void lcd_clr_to_eol(void);
extern void lcd_append(uint8_t chr);
extern void wr_lcd_num_chr(char *str,uint8_t noc);
extern uint32_t gettime(void);
extern void wait_ms(register uint32_t par);
extern void h2l(u16 val,uint8_t len);
extern void put_chr(uint8_t x,uint8_t y,uint8_t ch_val);
extern int putchar__(int chr);
extern void clrscr(void);
extern void lcd_cursor_off(void);
extern void lcd_cursor_on(void);
#endif

