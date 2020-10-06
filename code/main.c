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
#include "ff.h"

FATFS cffs;
FRESULT res;
FIL file;

typedef enum {SHOW_RAD, SHOW_TIME, SHOW_STATS} state_t;
static state_t state = SHOW_RAD;

static key_t key0, key1, key2, key3;	

static uint32_t last_uptime = 0;
static uint8_t  last_millis = 0;
static uint8_t backlight_timer = 0;

char* __fastcall__ utoa (unsigned val, char* buf, int radix);
char* __fastcall__ ultoa (unsigned long val, char* buf, int radix);
char* __fastcall__ strcpy (char* s1, const char* s2);
char* __fastcall__ strcat (char* s1, const char* s2);
size_t __fastcall__ strlen (const char* s);

static void prepare_disp (void);
static void update_disp (void);
static void log_data (void);

static void key0_func (void);
static void key1_func (void);
static void key2_func (void);
static void key3_func (void);

int main (void) {
	char buf[32];
	
	port_write(0x91);
	
	//Initialize button structures
	key_init(&key0, BTN0, key0_func);
	key_init(&key1, BTN1, key1_func);
	key_init(&key2, BTN2, key2_func);
	key_init(&key3, BTN3, key3_func);
	
	CONF_8255 = 0x82;
    mc6840_init();
    m6242_init();
    mos6551_init();
	hd44780_init();
	//cfInit();
	
	feed_hungry_watchdog();
	res = f_mount(&cffs, "", 1);
	mos6551_puts("CF init: ");
	utoa(res, buf, 10);
	mos6551_puts(buf);
	mos6551_puts("\r\n");
	feed_hungry_watchdog();

	prepare_disp();
	
	CLI();
	
	while(1) {
		if (uptime() != last_uptime) {
			last_uptime = uptime();
			update_disp();	
		}
		
		if ( (uint8_t)(millis() - last_millis) > 12 ) {			//12x20ms
			last_millis = millis();
			port_tgl(0x84);										//Toggle both LEDs
			feed_hungry_watchdog();								// Reset watchdog
		}
		
		if (backlight_timer && ( (uint8_t)(millis()-backlight_timer) > 200 ) ) {
			port_set(BACKLIGHT_PIN);							//Turn the backlight off
			backlight_timer = 0;
		}	

		key_update(&key0);
		key_update(&key1);
		key_update(&key2);
		key_update(&key3);
							
		mos6551_handle_rx();
		log_data();
	}
	
	return 0;
}


static void prepare_disp (void) {
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


static void update_disp (void) {
	uint16_t cpmin;
	char buffer[32];
	
	switch (state) {
		case SHOW_RAD:
		cpmin = get_geiger_pulses();
		get_usiv_str(cpmin, buffer);
		hd44780_gotoxy(1, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(1, 0);
		hd44780_puts(buffer);
		hd44780_puts(" uS/h");
		utoa(cpmin, buffer, 10);
		hd44780_gotoxy(2, 0);
		hd44780_puts("                    ");
		hd44780_gotoxy(2, 0);
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


static void log_data (void) {
    
    static uint32_t timer = 0;
    uint16_t cpmin;
	uint16_t bytes_written;
    char buffer[32];
    
    //Not enpugh samples
    if (uptime() < 60) return;
    
    if ( (uint32_t)(uptime()-timer) >= 120 ) {
        timer = uptime();
        
		res = f_open(&file, "GEIGER.TXT", (FA_OPEN_APPEND | FA_WRITE));
		if (res != FR_OK) {
			return;
		}
		strcpy(buffer, m6242_read_time_str()); 
		f_write(&file, buffer, strlen(buffer), &bytes_written);
		f_write(&file, " ", 1, &bytes_written);
		strcpy(buffer, m6242_read_date_str()); 
		f_write(&file, buffer, strlen(buffer), &bytes_written);			
		f_write(&file, " - ", 3, &bytes_written);
		cpmin = get_geiger_pulses();
		get_usiv_str(cpmin, buffer);
		f_write(&file, buffer, strlen(buffer), &bytes_written);
		f_write(&file, " uS/h (", 6, &bytes_written);
		utoa(cpmin, buffer, 10);
		f_write(&file, buffer, strlen(buffer), &bytes_written);
		f_write(&file, " CPM)\r\n", 7, &bytes_written);
		f_close(&file);            
    }   
}


static void key0_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_RAD;
	prepare_disp();
	update_disp();
}


static void key1_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_TIME;
	prepare_disp();
	update_disp();
}


static void key2_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
	state = SHOW_STATS;
	prepare_disp();
	update_disp();
}


static void key3_func (void) {
	port_clr(BACKLIGHT_PIN);				//Turn the backlight on
	backlight_timer = millis();				//Set timer for backlight utomatic turn off
}
