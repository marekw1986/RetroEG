#include "mos6551.h"
#include "mc6840.h"
#include "parser.h"
#include "io.h"

#define ACIA_RXD (*(volatile uint8_t*)0x6500)  //ACIA receive data port
#define ACIA_TXD (*(volatile uint8_t*)0x6500)  //ACIA transmit data port
#define ACIA_STS (*(volatile uint8_t*)0x6501)  //ACIA status port
#define ACIA_RES (*(volatile uint8_t*)0x6501)  //ACIA reset port
#define ACIA_CMD (*(volatile uint8_t*)0x6502)  //ACIA command port
#define ACIA_CTL (*(volatile uint8_t*)0x6503)  //ACIA control port

#define MODBUS_FRAME_TIMEOUT 4

volatile char mos6551_RxChar;
volatile char mos6551_rxrb[256];				//DO NOT CHAGE! IT NEEDS TO BE 256 BYTES LONG!
volatile uint8_t mos6551_rxrb_head = 0;
volatile uint8_t mos6551_rxrb_tail = 0;
uint8_t mb_rx[256];
uint8_t mb_len;
uint8_t mb_last_rx_time = 0;

void __fastcall__ modbus_process_frame(uint8_t* buf, uint8_t buf_len);

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

void __fastcall__ mos6551_send(const uint8_t *buf, uint16_t len) {
    port_set(RS485_PIN);			//Set RS485 to transmit
    while(len--)
    {
        while(!(ACIA_STS & 0x10));
        ACIA_TXD = *buf++;
    }
    while(!(ACIA_STS & 0x10));
    port_clr(RS485_PIN);			//Set RS485 to receive again
}

void __fastcall__ mos6551_handle_rx(void) {
    while (mos6551_rxrb_head != mos6551_rxrb_tail) {
        mb_rx[mb_len++] = mos6551_rxrb[mos6551_rxrb_tail];
        mos6551_rxrb_tail++;

        mb_last_rx_time = millis();

        // Prevent overflow if something goes wrong
        if (mb_len == 255)
            mb_len = 0;
    }

    // No bytes pending.
    // If we have a frame and the bus has been idle long enough,
    // process it.
    if (mb_len)
    {
        if ((uint8_t)(millis() - mb_last_rx_time) > MODBUS_FRAME_TIMEOUT)
        {
            modbus_process_frame((uint8_t *)mb_rx, mb_len);

            mb_len = 0;
        }
    }
}

void __fastcall__ modbus_process_frame(uint8_t* buf, uint8_t buf_len) {
	
}
