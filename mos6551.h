#ifndef _MOS6551_H_
#define _MOS_6551_H_

#include <inttypes.h>

#define MOS6551_RX_BUF_SIZE 256

void mos6551_init (void);
void mos6551_putc (char c);
void mos6551_puts (const char *str);
void mos6551_handle_rx (void);

#endif //_MOS6551_H_
