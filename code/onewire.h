#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>

/* Returns 1 if a device asserted a presence pulse, 0 if the bus
 * is empty. */
uint8_t ow_reset(void);

void    ow_write_bit(uint8_t bit);
uint8_t ow_read_bit(void);

/* LSB first, per the 1-Wire spec */
void    ow_write_byte(uint8_t data);
uint8_t ow_read_byte(void);

#endif
