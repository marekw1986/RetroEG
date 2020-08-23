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


int8_t supbuf[15];
uint8_t i;
char* tok;


void parse_cmd(char * pBuf) {
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
	ultoa(uptime(), (char*)supbuf, 10);
	mos6551_puts((char*)supbuf);
	mos6551_puts("\r\n");
	return 1;	
}
