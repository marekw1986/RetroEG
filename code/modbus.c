#include <stdlib.h>
#include "modbus.h"
#include "mc6840.h"
#include "mos6551.h"
#include "m6242.h"

extern uint8_t mb_rx[256];
extern uint8_t mb_len;

#define MB_MAX_REGS  16
volatile uint16_t holding[MB_MAX_REGS];
volatile uint16_t input[MB_MAX_REGS];

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
        case 0x04: // Read Input Registers
        {
            uint16_t start = (mb_rx[2] << 8) | mb_rx[3];
            uint16_t qty   = (mb_rx[4] << 8) | mb_rx[5];
            uint8_t resp[64];
            uint8_t idx = 0;
            uint16_t i, reg, crc;

            if (qty == 0 || qty > MB_MAX_REGS) break;

            resp[idx++] = SLAVE_ADDR;
            resp[idx++] = 0x04;
            resp[idx++] = qty * 2;

            for (i = 0; i < qty; i++)
            {
                reg = input[start + i];

                resp[idx++] = reg >> 8;
                resp[idx++] = reg & 0xFF;
            }

            crc = modbus_crc(resp, idx);
            resp[idx++] = crc & 0xFF;
            resp[idx++] = crc >> 8;

            if (mb_rx[0] != 0) mos6551_send(resp, idx);

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
            if (mb_rx[0] != 0) mos6551_send(mb_rx, mb_len); // Do not respond to broadcast messages
            
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

			if (mb_rx[0] != 0)  mos6551_send(resp, 8); // Do not respond to broadcast messages
           
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

			if (mb_rx[0] != 0) mos6551_send(resp, 5); // Do not respond to broadcast messages
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

void modbus_set_cpm(void) {
	uint16_t cpm = get_geiger_pulses();
	input[MODBUS_INPUT_CPM] = cpm;
}

uint16_t modbus_get_cpm(void) {
	return input[MODBUS_INPUT_CPM];
}

void modbus_set_sivert(void) {
	uint32_t siv = GEIGER_USV(input[MODBUS_INPUT_CPM]);
	input[MODBUS_INPUT_SIVERT_INT] = siv/10000;
	input[MODBUS_INPUT_SIVERT_FRACT] = siv%10000;	
}

volatile uint16_t* modbus_get_sivert(void) {
	return &input[MODBUS_INPUT_SIVERT_INT];
}

void modbus_set_time(void) {
	time_t* timestamp = m6242_read_timestamp();
	input[MODBUS_INPUT_TIMEH] = (uint16_t)(*timestamp >> 16);
	input[MODBUS_INPUT_TIMEL] = (uint16_t)(*timestamp & 0xFFFF);
}

void modbus_set_uptime(void) {
	uint32_t upt = uptime();
	input[MODBUS_INPUT_UPTIMEH] = (uint16_t)(upt >> 16);
	input[MODBUS_INPUT_UPTIMEL] = (uint16_t)(upt & 0xFFFF);
}
