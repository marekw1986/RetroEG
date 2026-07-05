#include "bus8255.h"
#include "i2c.h"

/* Half-bit delay in delay_16us() units. One unit (~16us) gives a bus
 * period around 30-40us once you add the 8255 access overhead on
 * each side - comfortably under the 100kHz standard-mode max, and
 * well above its minimum SCL high/low times (4.0/4.7us), so there's
 * plenty of margin either way. Bump this up if you want more margin,
 * or drop to 0 (no explicit delay - the code's own overhead at 1MHz
 * is already well under 100kHz on its own) if you want it faster. */
#define I2C_HALFBIT_UNITS 1

static void sda_low(void)     { porta_shadow &= (uint8_t)~BIT_PA_SDA_OUT; PA_8255 = porta_shadow; }
static void sda_release(void) { porta_shadow |=  BIT_PA_SDA_OUT;          PA_8255 = porta_shadow; }
static void scl_low(void)     { porta_shadow &= (uint8_t)~BIT_PA_SCL_OUT; PA_8255 = porta_shadow; }
static void scl_release(void) { porta_shadow |=  BIT_PA_SCL_OUT;          PA_8255 = porta_shadow; }

static uint8_t sda_read(void) {
    return (PB_8255 & BIT_PB_SDA_IN) ? 1 : 0;
}

/*
 * NOTE: there is no SCL_IN on this board, so this driver cannot
 * detect clock stretching - scl_release() just asserts the line and
 * moves on after a fixed delay, it never checks whether the line
 * actually made it high. That's fine for slaves that never stretch
 * (most simple sensors/EEPROMs/RTCs), but it will misbehave against
 * anything that holds SCL low to pace the master. If you hit one of
 * those, you'll need to wire SCL back into a spare Port B input bit
 * and add a "poll until SCL reads high" loop in scl_release().
 */

void i2c_start(void) {
    sda_release();
    scl_release();
    delay_16us(I2C_HALFBIT_UNITS);
    sda_low();
    delay_16us(I2C_HALFBIT_UNITS);
    scl_low();
}

void i2c_stop(void) {
    sda_low();
    delay_16us(I2C_HALFBIT_UNITS);
    scl_release();
    delay_16us(I2C_HALFBIT_UNITS);
    sda_release();
    delay_16us(I2C_HALFBIT_UNITS);
}

static void i2c_write_bit(uint8_t bit) {
    if (bit) {
        sda_release();
    } else {
        sda_low();
    }
    delay_16us(I2C_HALFBIT_UNITS);
    scl_release();
    delay_16us(I2C_HALFBIT_UNITS);
    scl_low();
}

static uint8_t i2c_read_bit(void) {
    uint8_t bit;
    sda_release();      /* let the slave (or the pull-up) drive SDA */
    delay_16us(I2C_HALFBIT_UNITS);
    scl_release();
    delay_16us(I2C_HALFBIT_UNITS);
    bit = sda_read();
    scl_low();
    return bit;
}

uint8_t i2c_write_byte(uint8_t data) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        i2c_write_bit(data & 0x80);
        data <<= 1;
    }
    return !i2c_read_bit();   /* ACK = slave pulls SDA low */
}

uint8_t i2c_read_byte(uint8_t ack) {
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++) {
        data <<= 1;
        data |= i2c_read_bit();
    }
    i2c_write_bit(!ack);      /* master drives the ACK/NACK bit */
    return data;
}
