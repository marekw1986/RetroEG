#include "mos6551.h"
#include "parser.h"
#include "io.h"

#define ACIA_RXD (*(volatile uint8_t*)0x6500)  //ACIA receive data port
#define ACIA_TXD (*(volatile uint8_t*)0x6500)  //ACIA transmit data port
#define ACIA_STS (*(volatile uint8_t*)0x6501)  //ACIA status port
#define ACIA_RES (*(volatile uint8_t*)0x6501)  //ACIA reset port
#define ACIA_CMD (*(volatile uint8_t*)0x6502)  //ACIA command port
#define ACIA_CTL (*(volatile uint8_t*)0x6503)  //ACIA control port

volatile char mos6551_RxChar;
volatile char mos6551_rxrb[256];				//DO NOT CHAGE! IT NEEDS TO BE 256 BYTES LONG!
volatile uint8_t mos6551_rxrb_head = 0;
volatile uint8_t mos6551_rxrb_tail = 0;
char mos6551_line[256];		//DO NOT CHAGE! IT NEEDS TO BE 256 BYTES LONG! Oftherwise uncomment and adjust line in mos6551_handle_rx
uint8_t mos6551_line_ind = 0;

void __fastcall__ mos6551_init (void) {
	//initialise 6551 ACIA
    ACIA_RES = 0xFF;		//soft reset (value not important)
    //ACIA_CMD = 0x0B;    	/set specific modes and functions: no parity, no echo, no Tx interrupt, no Rx interrupt, enable Tx/Rx
	ACIA_CMD = 0x09;     	//set specific modes and functions: no parity, no echo, no Tx interrupt, Rx interrupt enabled, enable Tx/Rx  
    ACIA_CTL = 0x1E;    	//8-N-1, 9600 baud
    port_clr(RS485_PIN);	//RS485 set to receive by default
}


void __fastcall__ mos6551_putc (char c) {
	while (!(ACIA_STS & 0x10));
	ACIA_TXD = c;
}


void __fastcall__ mos6551_puts (const char *str) {
	port_set(RS485_PIN);			//Set RS485 to transmit
	while (*str != '\0') {
		while (!(ACIA_STS & 0x10));
		ACIA_TXD = *str;
		str++;
	}
	while (!(ACIA_STS & 0x10));		//Be it is not transmitting before switching back to receceive
	port_clr(RS485_PIN);			//Set RS485 to receive again
}

void __fastcall__ mos6551_handle_rx (void) {
	while (mos6551_rxrb_head != mos6551_rxrb_tail) {			// There is a new data in ring buffer
		mos6551_RxChar = mos6551_rxrb[mos6551_rxrb_tail];
		mos6551_rxrb_tail++;
		switch(mos6551_RxChar) {
			case 0:  break;			// ignorujemy znak \0
			case 13: break;			// ignorujemy znak CR
			
			case 10:
			mos6551_line[mos6551_line_ind] = '\0';
			mos6551_line_ind = 0;
			parse_cmd(mos6551_line);
			break;
			
			default:
			mos6551_line[mos6551_line_ind] = mos6551_RxChar;
			mos6551_line_ind++;
			//if (mos6551_line_ind >= MOS6551_LINE_BUF_LEN) mos6551_line_ind = 0;		//Not needed if buffer size is 256
			break;
		}	
	}
}
