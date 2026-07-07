#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "modbus.h"
#include "mc6840.h"
#include "mos6551.h"
#include "m6242.h"

extern uint8_t mb_rx[256];
extern uint8_t mb_len;

#define MB_MAX_REGS  16
volatile uint16_t holding[MB_MAX_REGS];
volatile uint16_t input[MB_MAX_REGS] = {0x1234, 0x5678, 0xFFFF, 0xFAFA};

extern time_t timestamp;

void modbus_init(void) {
    memset(holding, 0x00, sizeof(holding));
    memset(input, 0x00, sizeof(input));
}

uint16_t modbus_crc(const uint8_t *buf, uint16_t len);
void modbus_apply_if_needed(uint16_t reg);
void modbus_apply_command(void);

void modbus_process_frame(void) {
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

            modbus_apply_command();

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
            
            modbus_apply_command();

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

uint16_t modbus_crc(const uint8_t *buf, uint16_t len) {
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

void modbus_apply_if_needed(uint16_t reg) {
	
}

void modbus_apply_command(void)
{
	switch(holding[MODBUS_HOLDING_CMD]) {
		case MODBUS_CMD_SET_TIME:
		{
			uint8_t hours = (uint8_t)(holding[MODBUS_H1] >> 8);
			uint8_t minutes = (uint8_t)holding[MODBUS_H1];
			uint8_t seconds = (uint8_t)holding[MODBUS_H2];
			if ((hours < 24) && (minutes < 60) && (seconds < 60)) {
				m6242_settime(hours, minutes, seconds);
			}
			break;
		}
		case MODBUS_CMD_SET_DATE:
		{
			uint8_t day = (uint8_t)(holding[MODBUS_H1] >> 8);
			uint8_t month = (uint8_t)holding[MODBUS_H1];
			uint8_t year = (uint8_t)holding[MODBUS_H2];
			if ((day > 0) && (day <= 31) && (month > 0) && (month <= 12) && (year < 100)) {
				m6242_setdate(day, month, year);
			}
			break;
		}
		case MODBUS_CMD_SET_DATETIME:
		{
			uint8_t hours = (uint8_t)(holding[MODBUS_H1] >> 8);
			uint8_t minutes = (uint8_t)holding[MODBUS_H1];
			uint8_t seconds = (uint8_t)(holding[MODBUS_H2] >> 8);
			uint8_t day = (uint8_t)holding[MODBUS_H2];
			uint8_t month = (uint8_t)(holding[MODBUS_H3] >> 8);
			uint8_t year = (uint8_t)holding[MODBUS_H3];
			if ((hours < 24) && (minutes < 60) && (seconds < 60) && (day > 0) && (day <= 31) &&
				(month > 0) && (month <= 12) && (year < 100)) {
				m6242_setdate(day, month, year);
				m6242_settime(hours, minutes, seconds);
			}
			break;
		}
		case MODBUS_CMD_SET_TIMEZONE:
		{
			EEConfig.timezone = (int8_t)holding[MODBUS_H1];
			break;
		}
        case MODBUS_CMD_SET_DST:
        {
            EEConfig.dst = (uint8_t)holding[MODBUS_H1];
        }
		default:
			break;
	}
	holding[MODBUS_HOLDING_CMD] = 0x00;
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

void modbus_set_ds18b20_temp(int32_t val) {
	int16_t integer = val / 1000;
	uint16_t fraction = abs(val % 1000);
    input[MODBUS_INPUT_DSTEMPH] = (uint16_t)integer;
    input[MODBUS_INPUT_DSTEMPL] = (uint16_t)fraction;
    input[MODBUS_INPUT_DSTEMP_TIMEH] = (uint16_t)(timestamp >> 16);
    input[MODBUS_INPUT_DSTEMP_TIMEL] = (uint16_t)(timestamp & 0xFFFF);    
}

volatile uint16_t* modbus_ds18b20_temp(void) {
	return &input[MODBUS_INPUT_DSTEMPH];
}

void modbus_set_time(void) {
	time_t* tmstmp = m6242_read_timestamp();
	input[MODBUS_INPUT_TIMEH] = (uint16_t)(*tmstmp >> 16);
	input[MODBUS_INPUT_TIMEL] = (uint16_t)(*tmstmp & 0xFFFF);
}

void modbus_set_uptime(void) {
	uint32_t upt = uptime();
	input[MODBUS_INPUT_UPTIMEH] = (uint16_t)(upt >> 16);
	input[MODBUS_INPUT_UPTIMEL] = (uint16_t)(upt & 0xFFFF);
}

void modbus_set_cf_time() {
	input[MODBUS_INPUT_LASTCFH] = (uint16_t)(timestamp >> 16);
	input[MODBUS_INPUT_LASTCFL] = (uint16_t)(timestamp & 0xFFFF);    
}

void modbus_set_cf_result(uint8_t res) {
	input[MODBUS_INPUT_CFRES] = res;
}
