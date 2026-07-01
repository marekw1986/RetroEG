#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "parser.h"
#include "mc6840.h"
#include "mos6551.h"
#include "m6242.h"

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


static uint8_t settime_service(char * params) {
	char* tok;
	int8_t supbuf[4];
	
	if (!params) return 0;
	
	tok = strtok(params, ":");
	if (!tok) return 0;
	supbuf[0] = atoi(tok);
	if ( (supbuf[0] < 1) || (supbuf[0] > 23) ) return 0;
	tok = strtok(NULL, ":");
	if (!tok) return 0;
	supbuf[1] = atoi(tok);
	if ( (supbuf[1] < 0) || (supbuf[1] > 59) ) return 0;
	tok = strtok(NULL, ":");
	if (!tok) return 0;
	supbuf[2] = atoi(tok);
	if ( (supbuf[2] < 0) || (supbuf[2] > 59) ) return 0;
	
	m6242_settime(supbuf[0], supbuf[1], supbuf[2]);
	//mos6551_puts("Godzina: %d, Minuta: %d, Sekunda: %d\r\n", supbuf[0], supbuf[1], supbuf[2]);
	
	return 1;	
}


static uint8_t gettime_service(char * params) {
	mos6551_puts(m6242_read_time_str());
	mos6551_puts("\r\n");
	return 1;	
}


static uint8_t setdate_service(char * params) {
	char* tok;
	int8_t supbuf[4];
	
	if (!params) return 0;	
	
	tok = strtok(params, "-");
	if (!tok) return 0;
	supbuf[0] = atoi(tok);
	if ( (supbuf[0] < 1) || (supbuf[0] > 31) ) return 0;
	tok = strtok(NULL, "-");
	if (!tok) return 0;
	supbuf[1] = atoi(tok);
	if ( (supbuf[1] < 1) || (supbuf[1] > 12) ) return 0;
	tok = strtok(NULL, "-");
	if (!tok) return 0;
	supbuf[2] = atoi(tok);
	if ( (supbuf[2] < 0) || (supbuf[2] > 99) ) return 0;
	
	m6242_setdate(supbuf[0], supbuf[1], supbuf[2]);
	//mos6551_puts("Dzien: %d, Miesiac: %d, Rok: %d\r\n", supbuf[0], supbuf[1], supbuf[2]);
	
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
