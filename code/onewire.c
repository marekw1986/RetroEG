#include "bus8255.h"
#include "onewire.h"

/* Drive the bus low (turn the PA3 -> 7407 driver on) */
static void ow_low(void) {
    porta_shadow &= (uint8_t)~BIT_PA_1W_OUT;
    PA_8255 = porta_shadow;
}

/* Release the bus (driver off, external pull-up takes it high) */
static void ow_release(void) {
    porta_shadow |= BIT_PA_1W_OUT;
    PA_8255 = porta_shadow;
}

/* Sample the bus level via the separate PB2 input */
static uint8_t ow_sample(void) {
    return (PB_8255 & BIT_PB_1W_IN) ? 1 : 0;
}

uint8_t ow_reset(void) {
    uint8_t presence;

    ow_low();
    delay_16us(30);           /* 480us reset pulse (spec minimum)     */
    ow_release();
    delay_16us(5);             /* ~80us into the presence-detect window */
    presence = !ow_sample();   /* device pulls the bus low if present  */
    delay_16us(26);            /* ~416us to round out the recovery time */

    return presence;
}

/*
 * Bit timing below relies on the CPU running at 1MHz (1 cycle = 1us).
 * The '1'-bit and read-bit low pulses use NO explicit delay between
 * the low and release calls: the AND/STA then OR/STA pair already
 * takes on the order of a few microseconds at 1MHz, which is exactly
 * the 1-15us window those slots want. Everything >=16us goes through
 * delay_16us(). Check the low-pulse widths on a scope once - if your
 * actual build's cycle count for ow_low()/ow_release() lands outside
 * 1-15us, that's the only place in this file that would need hand
 * tuning (e.g. an inline NOP or two).
 */
void ow_write_bit(uint8_t bit) {
    ow_low();
    if (bit) {
        ow_release();
        delay_16us(4);          /* ~64us, rounds out a ~70-80us slot */
    } else {
        delay_16us(4);           /* hold low ~64us (spec wants >=60us) */
        ow_release();
        delay_16us(1);           /* ~16us recovery before next slot */
    }
}

uint8_t ow_read_bit(void) {
    uint8_t bit;

    ow_low();
    ow_release();
    bit = ow_sample();           /* sampled well inside the 15us window */
    delay_16us(4);                /* finish out the rest of the slot */

    return bit;
}

void ow_write_byte(uint8_t data) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        ow_write_bit(data & 0x01);
        data >>= 1;
    }
}

uint8_t ow_read_byte(void) {
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++) {
        data >>= 1;
        if (ow_read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}
