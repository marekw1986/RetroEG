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
//#include "ff.h"

typedef enum {SHOW_RAD, SHOW_TIME, SHOW_STATS} state_t;
static state_t state = SHOW_RAD;

//FATFS CFFatFS;
static char buffer[64];
static uint32_t change_state_timer = 0;
static uint32_t last_uptime = 0;
static uint8_t  last_millis = 0;
static int16_t integer;
static uint16_t fraction, cpmin;
static uint32_t siv;

key_t key0, key1, key2;	

char* __fastcall__ utoa (unsigned val, char* buf, int radix);
char* __fastcall__ ultoa (unsigned long val, char* buf, int radix);
size_t __fastcall__ strlen (const char* s);

void prepare_disp (void);
void update_disp (void);

void key0_func (void);
void key1_func (void);
void key2_func (void);

int main (void) {
	
//	FRESULT res;
	
	port_write(0x80);
	
	CONF_8255 = 0x82;
    mc6840_init();
    m6242_init();
    mos6551_init();
	hd44780_init();

	prepare_disp();
	key_init(&key0, BTN0, key0_func);
	key_init(&key1, BTN1, key1_func);
	key_init(&key2, BTN2, key2_func);
	
	CLI();
	
//	res = f_mount(&CFFatFS, "0:", 1);
//    if (res != FR_OK) {
		//Dodać raportowanie przez UART
//	}
//    else {
		//Dodać raportowanie przez UART
//	}
	
	while(1) {
		if (uptime() != last_uptime) {
			last_uptime = uptime();
			if ( (uint32_t)(uptime() - change_state_timer) > 10 ) {
				change_state_timer = uptime();
				state++;
				if (state > SHOW_STATS) { state = SHOW_RAD; }
				prepare_disp();
			}
			update_disp();	
		}
		
		if ( (uint8_t)(millis() - last_millis) > 12 ) {			//12x20ms
			last_millis = millis();
			port_tgl(0x85);										//Toggle both LEDs and watchgod line
		}
//		key_update(&key0);	
//		key_update(&key1);	
//		key_update(&key2);			
		mos6551_handle_rx();
	}
	
	return 0;
}


void prepare_disp (void) {
	hd44780_clrscr();
	switch (state) {
		case SHOW_RAD:
		hd44780_gotoxy(0, 0);
		hd44780_write("Promieniowanie", 14);
		break;
		
		case SHOW_TIME:
		hd44780_gotoxy(0, 0);
		hd44780_write("Czas", 4);		
		break;
		
		case SHOW_STATS:
		hd44780_gotoxy(0, 0);
		hd44780_write("Stats", 5);		
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
		hd44780_write("                    ", 20);
		hd44780_gotoxy(1, 0);
		itoa(integer, buffer, 10);
		hd44780_write(buffer, strlen(buffer));
		hd44780_write(".", 1);
		if (fraction < 1000) {
			hd44780_write("0", 1);
			if (fraction < 100) {
				hd44780_write("0", 1);
				if (fraction < 10) {
					hd44780_write("0", 1);
				}
			}
		}
		utoa(fraction, buffer, 10);
		hd44780_write(buffer, strlen(buffer));
		hd44780_write(" uS/h", 5);
		hd44780_gotoxy(2, 0);
		hd44780_write("                    ", 20);
		hd44780_gotoxy(2, 0);
		utoa(cpmin, buffer, 10);
		hd44780_write(buffer, strlen(buffer));
		hd44780_write(" CPM", 4);		
		break;
		
		case SHOW_TIME:
		hd44780_gotoxy(1, 0);
		hd44780_write("                    ", 20);
		hd44780_gotoxy(1, 0);
		hd44780_write(m6242_read_time_str(), 8);
		hd44780_gotoxy(2, 0);
		hd44780_write("                    ", 20);
		hd44780_gotoxy(2, 0);
		hd44780_write(m6242_read_date_str(), 8);		
		break;
		
		case SHOW_STATS:
		hd44780_gotoxy(1, 0);
		hd44780_write("                    ", 20);
		hd44780_gotoxy(1, 0);
		hd44780_write("U: ", 3);
		ultoa(uptime(), buffer, 10);
		hd44780_write(buffer, strlen(buffer));
        hd44780_gotoxy(2, 0);
		hd44780_write("                    ", 20);
		hd44780_gotoxy(2, 0);
        hd44780_write("B: ", 3);
        utoa(BTNS, buffer, 10);
        hd44780_write(buffer, strlen(buffer));		
		break;
		
	}
}


void key0_func (void) {
    port_tgl(0x80);
	//state = SHOW_RAD;
	//prepare_disp();
	//update_disp();
}


void key1_func (void) {
    port_tgl(0x80);
	//state = SHOW_TIME;
	//prepare_disp();
	//update_disp();
}


void key2_func (void) {
    port_tgl(0x10);
	//state = SHOW_STATS;
	//prepare_disp();
	//update_disp();
}

/*
void handle_cf_log (void) {
    
    static uint32_t timer = 0;
    static FRESULT res;
    static FIL file;
    time_t tm;
    static int16_t integer;
	static uint16_t fraction, siv, countspm;
    static char temp[100];
    
    if (((uint32_t)(uptime()-timer)) >= 30 ) {
        timer = uptime();

		res = f_open(&file, "1:/geiger.csv", (FA_OPEN_ALWAYS | FA_WRITE));
		if (res != FR_OK) {
			//printf("f_open error code: %i\r\n", res);
			return;
		}
		if (f_size(&file) == 0) {
			f_puts("Time (UTC);CPM;uSv/h\r\n", &file);
		}
		else {
			res = f_lseek(&file, f_size(&file));
			if (res != FR_OK) {
				//printf("f_lseek error code: %i\r\n", res); 
				f_close(&file);
				return;
			}
		}
		
		countspm = cpm();
		siv = cpm2sievert(countspm);
		integer = siv/10000;
		fraction = abs(siv%10000); 
		//tm = rtccGetTimestamp();
		//strncpy(temp, asctime(gmtime(&tm)), sizeof(temp)-1);
		//strtok(temp, "\n");
		//f_puts(temp, &file);
		//f_putc(';', &file);
		sprintf(temp, "%d", countspm);
		f_puts(temp, &file);
		f_putc(';', &file);
		sprintf(temp, "%i.%.4i\r\n", integer, fraction);
		f_puts(temp, &file);
		f_close(&file);            
    }   
}
*/
