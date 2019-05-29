#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "config.h"
#include "hd44780.h"
#include "delay.h"
#include "ff.h"
#include "time.h"
#include "geiger.h"

//0x6000 - CS0
//0x6300 - BUTTONS
//0x6400 - LCD
//0x7C00 - 373
#define PORT (*(volatile uint8_t*)0x7C00)
#define BUTTONS (*(volatile uint8_t*)0x6300)

//extern cfg_t Config;

FATFS CFFatFS;
uint16_t zmienna = 0;
char buffer[64];

char* __fastcall__ utoa (unsigned val, char* buf, int radix);
size_t __fastcall__ strlen (const char* s);


void hd44780_write (uint8_t* buf, uint8_t len) {
	uint8_t i;
	for (i=0; i<len; i++) {
		hd44780_putc(buf[i]);
	}
}


int main (void) {
	
	FRESULT res;
	
	
	PORT = 0x84;
	hd44780_init();
	hd44780_write("6502 is still alive!", 20);
	hd44780_gotoxy(1, 0);
	hd44780_write("Device designed by", 18);
	hd44780_gotoxy(2, 0);
	hd44780_write("Marek Wiecek SQ9RZI", 19);
	
	res = f_mount(&CFFatFS, "0:", 1);
    if (res != FR_OK) {
		//Dodać raportowanie przez UART
	}
    else {
		//Dodać raportowanie przez UART
	}
	
	while(1) {
		hd44780_gotoxy(3, 0);
		hd44780_write("     ", 5);
		hd44780_gotoxy(3, 0);
		zmienna++;
		utoa(zmienna, buffer, 10);
		hd44780_write(buffer, strlen(buffer));
		
		PORT = 0x80;
		delay_ms(250);
		
		if (!(BUTTONS & 0x04)) {
			zmienna += 5;
		}
		
		PORT = 0x05;
		delay_ms(250);

	}
	
	return 0;
}

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
