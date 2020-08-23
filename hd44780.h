#ifndef _HD44780_H_
#define _HD44780_H_

#include <stdint.h>

void hd44780_init (void);
void hd44780_putc (char c);
void hd44780_write (uint8_t* buf, uint8_t len);
void hd44780_puts (const char *str);
void hd44780_cmd (uint8_t cmd);
void hd44780_gotoxy (uint8_t x, uint8_t y);
void hd44780_clrscr (void);

#endif
