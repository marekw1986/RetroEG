#ifndef _MOS6551_H_
#define _MOS_6551_H_

#include <inttypes.h>

#define SLAVE_ADDR 0xAB

void __fastcall__ mos6551_init (void);
void __fastcall__ mos6551_send(const uint8_t *buf, uint16_t len);
void __fastcall__ mos6551_handle_rx (void);

#endif //_MOS6551_H_
