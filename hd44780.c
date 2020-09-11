#include "hd44780.h"
#include "delay.h"

#define HD_CMD		(*(uint8_t*)0x6380)
#define HD_DATA		(*(uint8_t*)0x6381)

void __fastcall__ hd44780_init (void) {
	//delay 100ms
	delay_ms(100);
	HD_CMD = 0x30;
	delay_ms(100);
	HD_CMD = 0x30;
	delay_16us(7);
	HD_CMD = 0x30;
	delay_16us(7);
	hd44780_cmd(0x38);
	hd44780_cmd(0x08);
	hd44780_cmd(0x01);
	hd44780_cmd(0x06);
	hd44780_cmd(0x0C);	
}


void __fastcall__ hd44780_putc (char c) {
	while (HD_CMD & 0x80);
	HD_DATA = c;
}	

void __fastcall__ hd44780_write (uint8_t* buf, uint8_t len) {
	uint8_t ind;
	for (ind=0; ind<len; ind++) {
		hd44780_putc(buf[ind]);
	}
}

void __fastcall__ hd44780_puts (const char *str) {
	while (*str != '\0') {
		hd44780_putc(*str);
		str++;
	}
}


void __fastcall__ hd44780_cmd (uint8_t cmd) {
	while (HD_CMD & 0x80);
	HD_CMD = cmd;
}


void __fastcall__ hd44780_gotoxy (uint8_t x, uint8_t y) {
	uint8_t address = 0;
	if (x==0) {
		address = 0x80;
	} 
	else if (x==1) {
		address = 0xC0;
	}
	else if (x==2) {
		address = 0x94;
	}
	else if (x==3) {
		address = 0xD4;
	}
	if (y<20)
		address += y;
	hd44780_cmd(address);	
}


void __fastcall__ hd44780_clrscr (void) {
	uint8_t i;
	
	for (i=0; i<4; i++) {
		hd44780_gotoxy(i, 0);
		hd44780_puts("                    ");
	}
}

