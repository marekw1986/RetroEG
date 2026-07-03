#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>

enum modbus_input_idx {MODBUS_INPUT_CPM=0, MODBUS_INPUT_SIVERT};
enum modbus_holding_idx {MODBUS_HOLDING_CMD};

#define MODBUS_CMD_SET_TIME 	0x01
#define MODBUS_CMD_SET_DATE 	0x02
#define MODBUS_CMD_SETDATETIME 	0x03

void __fastcall__ modbus_process_frame(void);
uint16_t __fastcall__ modbus_crc(const uint8_t *buf, uint16_t len);
void __fastcall__ modbus_apply_if_needed(uint16_t reg);

void modbus_set_cpm(void);
uint16_t modbus_get_cpm();
void modbus_update_sivert(uint16_t siv);

#endif
