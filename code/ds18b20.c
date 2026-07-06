#include "onewire.h"
#include "ds18b20.h"

#define DS18B20_CMD_SKIP_ROM     0xCC
#define DS18B20_CMD_CONVERT_T    0x44
#define DS18B20_CMD_READ_SCRATCH 0xBE

uint8_t ds18b20_read_temp(int32_t *millic) {
    uint8_t lsb, msb;
    int16_t raw;

    if (!ow_reset()) {
        return 0;                    /* nothing answered the bus reset */
    }
    ow_write_byte(DS18B20_CMD_SKIP_ROM);
    ow_write_byte(DS18B20_CMD_CONVERT_T);

    /*
     * An externally-powered DS18B20 pulls the bus low while it's
     * converting and lets it float high (reads as 1) the instant
     * it's done, so we poll instead of guessing at a fixed delay.
     * Worst case (12-bit resolution, the power-on default) this
     * takes up to ~750ms.
     *
     * If your sensor is parasite-powered instead (Vdd tied to
     * GND, drawing power from the data line), this loop will
     * never see a 1 - the bus has to be held high with a strong
     * pull-up during conversion instead. In that case, replace
     * this loop with a fixed wait of your own, e.g.:
     *     for (i = 0; i < 47; i++) delay_16us(255);  // ~750ms-ish, tune it
     */
    while (ow_read_bit() == 0) {
        /* waiting for conversion to complete */
    }

    if (!ow_reset()) {
        return 0;
    }
    ow_write_byte(DS18B20_CMD_SKIP_ROM);
    ow_write_byte(DS18B20_CMD_READ_SCRATCH);

    lsb = ow_read_byte();
    msb = ow_read_byte();
    /* Scratchpad bytes 2-8 (alarm registers, config, CRC) are left
     * unread here. If you want to guard against a corrupted read
     * (line noise, a bad connection), read all 9 bytes and check
     * them against the CRC-8/MAXIM algorithm - happy to add that
     * if you want it. */

    raw = (int16_t)(((uint16_t)msb << 8) | lsb);  /* two's complement, 0.0625C/count */

    *millic = ((int32_t)raw * 625) / 10;   /* 0.0625C = 62.5 milli-C per count */

    return 1;
}
