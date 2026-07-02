#include "mos6551.h"
#include "mc6840.h"
#include "m6242.h"
#include "delay.h"
#include "parser.h"
#include "io.h"

#define ACIA_RXD (*(volatile uint8_t*)0x6500)  //ACIA receive data port
#define ACIA_TXD (*(volatile uint8_t*)0x6500)  //ACIA transmit data port
#define ACIA_STS (*(volatile uint8_t*)0x6501)  //ACIA status port
#define ACIA_RES (*(volatile uint8_t*)0x6501)  //ACIA reset port
#define ACIA_CMD (*(volatile uint8_t*)0x6502)  //ACIA command port
#define ACIA_CTL (*(volatile uint8_t*)0x6503)  //ACIA control port

#define SLAVE_ADDR 0xAB
#define MB_MAX_REGS  16
#define MODBUS_FRAME_TIMEOUT 4

volatile char mos6551_RxChar;
volatile char mos6551_rxrb[256];				//DO NOT CHAGE! IT NEEDS TO BE 256 BYTES LONG!
volatile uint8_t mos6551_rxrb_head = 0;
volatile uint8_t mos6551_rxrb_tail = 0;
volatile uint16_t holding[MB_MAX_REGS];
uint8_t mb_rx[256];
uint8_t mb_len;
uint16_t mb_last_rx_time = 0;

void __fastcall__ modbus_process_frame(void);
uint16_t __fastcall__ modbus_crc(const uint8_t *buf, uint16_t len);
void __fastcall__ modbus_apply_if_needed(uint16_t reg);

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
    delay_ms(2);
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
        if ((uint16_t)(millis() - mb_last_rx_time) > MODBUS_FRAME_TIMEOUT)
        {
            modbus_process_frame();
            mb_len = 0;
        }
    }
}

void __fastcall__ modbus_process_frame(void) {
    uint16_t crc_calc, crc_recv;
    uint8_t addr = mb_rx[0];
    uint8_t func = mb_rx[1];
    
    if (mb_len < 4) return; // too short
    if (addr != SLAVE_ADDR && addr != 0) return; // not for us (0 = broadcast)

    crc_calc = modbus_crc(mb_rx, mb_len - 2);
    crc_recv = mb_rx[mb_len - 2] | (mb_rx[mb_len - 1] << 8);

    if (crc_calc != crc_recv) return; // bad frame
    
    switch(func) {
        case 0x03:
        {
            uint8_t resp[64];
            uint8_t idx = 0;
            uint16_t i;
            uint16_t reg, crc;
            uint16_t start = (mb_rx[2] << 8) | mb_rx[3];
            uint16_t qty   = (mb_rx[4] << 8) | mb_rx[5];

            if (qty == 0 || qty > MB_MAX_REGS)
                break;
            resp[idx++] = SLAVE_ADDR;
            resp[idx++] = 0x03;
            resp[idx++] = qty * 2;

            
            for (i = 0; i < qty; i++)
            {
                reg = holding[start + i];

                resp[idx++] = reg >> 8;
                resp[idx++] = reg & 0xFF;
            }

            crc = modbus_crc(resp, idx);
            resp[idx++] = crc & 0xFF;
            resp[idx++] = crc >> 8;

			if (mb_rx[0] == 0) break; // Do not respond to broadcast messages
            mos6551_send(resp, idx);
            break;
        }
        case 0x06:
        {
            uint16_t reg = (mb_rx[2] << 8) | mb_rx[3];
            uint16_t val = (mb_rx[4] << 8) | mb_rx[5];

            if (reg < MB_MAX_REGS)
            {
                holding[reg] = val;

                // Example: commit hook
                modbus_apply_if_needed(reg);
            }

            // echo request as response (Modbus standard)
            if (mb_rx[0] == 0) break; // Do not respond to broadcast messages
            mos6551_send(mb_rx, mb_len);
            break;
        }
        case 0x10:
        {
            uint16_t start = (mb_rx[2] << 8) | mb_rx[3];
            uint16_t qty   = (mb_rx[4] << 8) | mb_rx[5];
            uint8_t bytecount = mb_rx[6];
            uint8_t *data = &mb_rx[7];
            uint16_t i, val, crc;
            // response = addr func start qty
            uint8_t resp[8];

            if (qty == 0 || qty > MB_MAX_REGS)
                break;

            for (i = 0; i < qty; i++)
            {
                val = (data[i*2] << 8) | data[i*2 + 1];
                holding[start + i] = val;

                modbus_apply_if_needed(start + i);
            }

            resp[0] = SLAVE_ADDR;
            resp[1] = 0x10;
            resp[2] = mb_rx[2];
            resp[3] = mb_rx[3];
            resp[4] = mb_rx[4];
            resp[5] = mb_rx[5];

            crc = modbus_crc(resp, 6);
            resp[6] = crc & 0xFF;
            resp[7] = crc >> 8;

			if (mb_rx[0] == 0) break; // Do not respond to broadcast messages
            mos6551_send(resp, 8);
            break;
        }
        default:
        {
            // Exception response: illegal function
            uint8_t resp[5];
            uint16_t crc;

            resp[0] = SLAVE_ADDR;
            resp[1] = mb_rx[1] | 0x80;
            resp[2] = 0x01; // illegal function

            crc = modbus_crc(resp, 3);
            resp[3] = crc & 0xFF;
            resp[4] = crc >> 8;

			if (mb_rx[0] == 0) break; // Do not respond to broadcast messages
            mos6551_send(resp, 5);
            break;
        }
    }
    mb_len = 0;
}

uint16_t __fastcall__ modbus_crc(const uint8_t *buf, uint16_t len) {
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t j;
    
    for (i = 0; i < len; i++)
    {
        crc ^= buf[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

void __fastcall__ modbus_apply_if_needed(uint16_t reg)
{
    if (reg == 5) // example: "commit register"
    {
        m6242_settime(
            holding[0],
            holding[1],
            holding[2]
        );
    }
}
