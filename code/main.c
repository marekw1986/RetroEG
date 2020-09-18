//0x4000 - EEPROM
//0x6000 - CS0		LCD + BUTTONS
//		0x6000 - NOT USED
//		0x6080 - NOT USED
//		0x6100 - NOT USED
//		0x6180 - NOT USED
//		0x6200 - NOT USED
//		0x6280 - NOT USED
//		0x6300 - BUTTONS
//		0x6380 - HD44780 LCD

//0x6400 - CS1
//		0x6400 - RTC
//		0x6480 - TIMER
//		0x6500 - UART
//		0x6580 - CF CARD
//		0x6600 - 8255
//		0x6680 - NOT USED
//		0x6700 - NOT USED
//		0x6780 - NOT USED

//0x6800 - CS2
//0x6C00 - CS3
//0x7000 - CS4
//0x7400 - CS5
//0x7800 - CS6
//0x7C00 - CS7

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <6502.h>
#include "config.h"
#include "hd44780.h"
#include "mos6551.h"
#include "mc6840.h"
#include "m6242.h"
#include "delay.h"
#include "parser.h"
#include "io.h"
#include "cf.h"

typedef enum {SHOW_RAD, SHOW_TIME, SHOW_STATS} state_t;
static state_t state = SHOW_RAD;

static char buffer[64];
static uint32_t last_uptime = 0;
static uint8_t  last_millis = 0;
static uint8_t backlight_timer = 0;
static int16_t integer;
static uint16_t fraction, cpmin;
static uint32_t siv;

static uint8_t key_press;
key_t key0, key1, key2, key3;	

char* __fastcall__ itoa (int val, char* buf, int radix);
char* __fastcall__ utoa (unsigned val, char* buf, int radix);
char* __fastcall__ ultoa (unsigned long val, char* buf, int radix);
size_t __fastcall__ strlen (const char* s);

void prepare_disp (void);
void update_disp (void);

void key0_func (void);
void key1_func (void);
void key2_func (void);
void key3_func (void);

int main (void) {
	
	port_write(0x90);
	
	//Initialize button structures
	key0.last_state = 0; key0.timer = 0;
	key1.last_state = 0; key1.timer = 0;
	key2.last_state = 0; key2.timer = 0;
	key3.last_state = 0; key3.timer = 0;
	
	CONF_8255 = 0x82;
    mc6840_init();
    m6242_init();
    mos6551_init();
	hd44780_init();

	prepare_disp();
	
	CLI();
	
	while(1) {
		if (uptime() != last_uptime) {
			last_uptime = uptime();
			update_disp();	
		}
		
		if ( (uint8_t)(millis() - last_millis) > 12 ) {			//12x20ms
			last_millis = millis();
			port_tgl(0x85);										//Toggle both LEDs and watchgod line
		}
		
		if (backlight_timer && ( (uint8_t)(millis()-backlight_timer) > 200 ) ) {
			port_set(BACKLIGHT_PIN);							//Turn the backlight off
			backlight_timer = 0;
		}	

		//Check and debounce BTN0
		key_press = !(BTNS & BTN0);	
		if (key_press != (key0.last_state)) {
			if (key_press) { key0.timer = millis(); }
			else { if ( (uint8_t)(millis()-(key0.timer)) > SHORT_WAIT ) key0_func(); }
			key0.last_state = key_press;
		}
		//Check and debounce BTN1
		key_press = !(BTNS & BTN1);	
		if (key_press != (key1.last_state)) {
			if (key_press) { key1.timer = millis(); }
			else { if ( (uint8_t)(millis()-(key1.timer)) > SHORT_WAIT ) key1_func(); }
			key1.last_state = key_press;
		}
		//Check and debounce BTN2
		key_press = !(BTNS & BTN2);	
		if (key_press != (key2.last_state)) {
			if (key_press) { key2.timer = millis(); }
			else { if ( (uint8_t)(millis()-(key2.timer)) > SHORT_WAIT ) key2_func(); }
			key2.last_state = key_press;
		}
		//Check and debounce BTN3
		key_press = !(BTNS & BTN3);	
		if (key_press != (key3.last_state)) {
			if (key_press) { key3.timer = millis(); }
			else { if ( (uint8_t)(millis()-(key3.timer)) > SHORT_WAIT ) key3_func(); }
			key3.last_state = key_press;
		}
							
		mos6551_handle_rx();
	}
	
	return 0;
}


void prepare_disp (void) {
	hd44780_clrscr();
	switch (state) {
		case SHOW_RAD:
		hd44780_gotoxy(0, 0);
		hd44780_puts("Promieniowanie");
		break;
		
		case SHOW_TIME:
		hd44780_gotoxy(0, 0);
		hd44780_puts("Czas");		
		break;
		
		case SHOW_STATS:
		hd44780_gotoxy(0, 0);
		hd44780_puts("Stats");		
		break;		
	}
}


void update_disp (void) {
	switch (state) {
		case SHOW_RAD:
		cpmin = get_geiger_pulses();
		siv = get_geiger_usv();
		integer = siv/10000;
		fraction = siv%10000;
		hd44780_gotoxy(1, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(1, 0);
		itoa(integer, buffer, 10);
		hd44780_puts(buffer);
		hd44780_putc('.');
		if (fraction < 1000) {
			hd44780_putc('0');
			if (fraction < 100) {
				hd44780_putc('0');
				if (fraction < 10) {
					hd44780_putc('0');
				}
			}
		}
		utoa(fraction, buffer, 10);
		hd44780_puts(buffer);
		hd44780_puts(" uS/h");
		hd44780_gotoxy(2, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(2, 0);
		utoa(cpmin, buffer, 10);
		hd44780_puts(buffer);
		hd44780_puts(" CPM");		
		break;
		
		case SHOW_TIME:
		hd44780_gotoxy(1, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(1, 0);
		hd44780_puts(m6242_read_time_str());
		hd44780_gotoxy(2, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(2, 0);
		hd44780_puts(m6242_read_date_str());		
		break;
		
		case SHOW_STATS:
		hd44780_gotoxy(1, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(1, 0);
		hd44780_puts("U: ");
		ultoa(uptime(), buffer, 10);
		hd44780_puts(buffer);					
		break;
		
	}
}


void key0_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_RAD;
	prepare_disp();
	update_disp();
}


void key1_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_TIME;
	prepare_disp();
	update_disp();
}


void key2_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_STATS;
	prepare_disp();
	update_disp();
}


void key3_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
}
