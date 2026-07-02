#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>

void __fastcall__ modbus_process_frame(void);
uint16_t __fastcall__ modbus_crc(const uint8_t *buf, uint16_t len);
void __fastcall__ modbus_apply_if_needed(uint16_t reg);

#endif
