#ifndef _MOS6551_H_
#define _MOS_6551_H_

#include <inttypes.h>

#define MOS6551_RX_BUF_SIZE 256

void __fastcall__ mos6551_init (void);
void __fastcall__ mos6551_putc (char c);
void __fastcall__ mos6551_puts (const char *str);
void __fastcall__ mos6551_handle_rx (void);

#endif //_MOS6551_H_
