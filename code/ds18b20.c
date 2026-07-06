#include "onewire.h"
#include "ds18b20.h"
#include "io.h"

#define DS18B20_CMD_SKIP_ROM     0xCC
#define DS18B20_CMD_CONVERT_T    0x44
#define DS18B20_CMD_READ_SCRATCH 0xBE

/* ~1 second worth of ow_read_bit() calls at ~80us each - see the
 * comment on the poll loop below. */
#define DS18B20_POLL_TIMEOUT_ITERS 12000

#define WDT_FEED()  feed_hungry_watchdog()

uint8_t ds18b20_read_temp(int32_t *millic) {
    uint8_t lsb, msb;
    int16_t raw;
    uint16_t timeout;

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
     * takes up to ~750ms - long enough to trip a watchdog if you
     * have one, so feed it on every iteration.
     *
     * DS18B20_POLL_TIMEOUT_ITERS bounds the loop independently of
     * the watchdog, so a wiring fault or dead sensor fails this
     * one reading instead of hanging forever. It's a rough count
     * (~1s worth of ow_read_bit() calls at ~80us each per the
     * timing in onewire.c) - tune it if your actual loop timing
     * ends up noticeably different.
     */
    for (timeout = 0; timeout < DS18B20_POLL_TIMEOUT_ITERS; timeout++) {
        if (ow_read_bit() != 0) {
            break;
        }
        WDT_FEED();   /* <-- replace with your board's actual watchdog feed call */
    }
    if (timeout == DS18B20_POLL_TIMEOUT_ITERS) {
        return 0;     /* conversion never completed - treat as a fault */
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
