#ifndef BUS8255_H
#define BUS8255_H

#include <stdint.h>
#include "io.h"

/* Mode 0, Port A = output, Port B = input, Port C = output (unused) */
#define CONF_8255_MODE0_A_OUT_B_IN   0x82

/* ---- Port A bit assignments (open-collector drivers via 7407) ---- */
#define BIT_PA_1W_OUT   0x08   /* PA3 - 1-Wire bus driver  */
#define BIT_PA_SDA_OUT  0x10   /* PA4 - I2C SDA driver     */
#define BIT_PA_SCL_OUT  0x40   /* PA6 - I2C SCL driver     */

/* ---- Port B bit assignments (bus sense inputs) ---- */
#define BIT_PB_1W_IN    0x04   /* PB2 - 1-Wire bus sense   */
#define BIT_PB_SDA_IN   0x08   /* PB3 - I2C SDA sense      */

/*
 * Shadow copy of the Port A output latch.
 *
 * PA3/PA4/PA6 drive open-collector 7407 buffers, so the *logical*
 * idle state is '1' (driver released, external pull-up holds the
 * bus high) and '0' actively pulls the corresponding bus line low.
 *
 * We keep a software shadow instead of reading PA_8255 back before
 * every read-modify-write. That avoids ever mixing bits belonging
 * to different buses, and avoids any dependence on whether this
 * particular 8255 part even returns the output latch (rather than
 * the pin state) on a read of an output-configured port.
 */
extern uint8_t porta_shadow;

void bus8255_init(void);

/*
 * Provided elsewhere in your codebase (1MHz 6502, ~16us per call
 * unit). Declared here so onewire.c/i2c_bitbang.c don't need an
 * extra include - delete this line if you'd rather pull the real
 * prototype in from wherever it's already declared.
 */
extern void __fastcall__ delay_16us(uint8_t n);

#endif
