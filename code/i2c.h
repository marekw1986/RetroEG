#ifndef I2C_BITBANG_H
#define I2C_BITBANG_H

#include <stdint.h>

void    i2c_start(void);
void    i2c_stop(void);

/* Returns 1 if the slave ACKed (SDA pulled low), 0 on NACK */
uint8_t i2c_write_byte(uint8_t data);

/* ack = 1 to ACK the received byte (more bytes to follow),
 * ack = 0 to NACK it (last byte of the transfer) */
uint8_t i2c_read_byte(uint8_t ack);

#endif
