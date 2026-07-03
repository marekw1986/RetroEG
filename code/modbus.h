#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>

enum modbus_input_idx {MODBUS_INPUT_CPM=0, MODBUS_INPUT_SIVERT_INT, MODBUS_INPUT_SIVERT_FRACT, MODBUS_INPUT_TIMEH, MODBUS_INPUT_TIMEL, MODBUS_INPUT_UPTIMEH, MODBUS_INPUT_UPTIMEL};
enum modbus_holding_idx {MODBUS_HOLDING_CMD=0, MODBUS_H1, MODBUS_H2, MODBUS_H3, MODBUS_H4, MODBUS_H5, MODBUS_H6};

#define MODBUS_CMD_SET_TIME 		0x01
#define MODBUS_CMD_SET_DATE 		0x02
#define MODBUS_CMD_SET_DATETIME 	0x03

void __fastcall__ modbus_process_frame(void);
uint16_t __fastcall__ modbus_crc(const uint8_t *buf, uint16_t len);
void __fastcall__ modbus_apply_if_needed(uint16_t reg);

void modbus_set_cpm(void);
uint16_t modbus_get_cpm(void);
void modbus_set_sivert(void);
volatile uint16_t* modbus_get_sivert(void);
void modbus_set_time(void);
void modbus_set_uptime(void);

#endif
