#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "parser.h"
#include "mc6840.h"
#include "mos6551.h"
#include "m6242.h"

char* __fastcall__ strtok (char* s1, const char* s2);
int __fastcall__ strcasecmp (const char* s1, const char* s2);
char* __fastcall__ strchr (const char* s, int c);
int __fastcall__ atoi (const char* s);
char* __fastcall__ ultoa (unsigned long val, char* buf, int radix);

static uint8_t settime_service(char * params);
static uint8_t gettime_service(char * params);
static uint8_t setdate_service(char * params);
static uint8_t getdate_service(char * params);
static uint8_t uptime_service(char * params);

#define CMD_CNT 	5

const TCMD komendy[CMD_CNT] = {
	 // { at_cmd } ,	{ wskaznik do funkcji obslugujacej komende },
		{"settime", 	settime_service},
		{"gettime", 	gettime_service},
		{"setdate", 	setdate_service},
		{"getdate", 	getdate_service},
		{"uptime",		uptime_service}
};


void parse_cmd(char * pBuf) {
	char* tok;
	uint8_t i;
	
	if ( strchr(pBuf, ' ') ) {			// Polecenia z paarametrami
		tok = strtok(pBuf, " \0");
		for(i=0;i<CMD_CNT;i++) {
			if ( 0 == strcasecmp(tok, komendy[i].komenda) ) {
				tok = strtok(NULL, " ");
				if (0 == komendy[i].cmd_service(tok)) { mos6551_puts("Invalid or missing parameter\r\n"); }
				else { mos6551_puts("OK\r\n"); }
				return;
			}
		}
		mos6551_puts("Unknown command\r\n");
	}
	else {							// Polecenia bez parametrów
		for(i=0;i<CMD_CNT;i++) {
			if ( 0 == strcasecmp(pBuf, komendy[i].komenda) ) {
				if (0 == komendy[i].cmd_service(NULL)) { mos6551_puts("Invalid or missing parameter\r\n"); }
				else { mos6551_puts("OK\r\n"); }
				return;
			}
		}
		mos6551_puts("Unknown command\r\n");
	}
}


static uint8_t settime_service(char * params) {
	char* tok;
	int8_t hours, minutes, seconds;
	
	if (!params) return 0;
	
	tok = strtok(params, ":");
	if (!tok) return 0;
	hours = atoi(tok);
	if ( (hours < 1) || (hours > 23) ) return 0;
	tok = strtok(NULL, ":");
	if (!tok) return 0;
	minutes = atoi(tok);
	if ( (minutes < 0) || (minutes > 59) ) return 0;
	tok = strtok(NULL, ":");
	if (!tok) return 0;
	seconds = atoi(tok);
	if ( (seconds < 0) || (seconds > 59) ) return 0;
	
	m6242_settime(hours, minutes, seconds);
	//mos6551_puts("Godzina: %d, Minuta: %d, Sekunda: %d\r\n", hours, minutes, seconds);
	
	return 1;	
}


static uint8_t gettime_service(char * params) {
	mos6551_puts(m6242_read_time_str());
	mos6551_puts("\r\n");
	return 1;	
}


static uint8_t setdate_service(char * params) {
	char* tok;
	int8_t day, month, year;
	
	if (!params) return 0;	
	
	tok = strtok(params, "-");
	if (!tok) return 0;
	day = atoi(tok);
	if ( (day < 1) || (day > 31) ) return 0;
	tok = strtok(NULL, "-");
	if (!tok) return 0;
	month = atoi(tok);
	if ( (month < 1) || (month > 12) ) return 0;
	tok = strtok(NULL, "-");
	if (!tok) return 0;
	year = atoi(tok);
	if ( (year < 0) || (year > 99) ) return 0;
	
	m6242_setdate(day, month, year);
	//mos6551_puts("Dzien: %d, Miesiac: %d, Rok: %d\r\n", day, month, year);
	
	return 1;	
}


static uint8_t getdate_service(char * params) {
	mos6551_puts(m6242_read_date_str());
	mos6551_puts("\r\n");
	return 1;
}


static uint8_t uptime_service(char * params) {
	char supbuf[15];
	
	ultoa(uptime(), supbuf, 10);
	mos6551_puts(supbuf);
	mos6551_puts("\r\n");
	return 1;	
}
