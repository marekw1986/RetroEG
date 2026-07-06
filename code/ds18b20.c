#include "mc6840.h"
#include "modbus.h"
#include "onewire.h"
#include "ds18b20.h"
#include "io.h"

#define DS18B20_CMD_SKIP_ROM     0xCC
#define DS18B20_CMD_CONVERT_T    0x44
#define DS18B20_CMD_READ_SCRATCH 0xBE

#define TICK_20S    20000
#define TICK_750MS  750

typedef enum {
    DS_IDLE,
    DS_CONVERTING,
    DS_READING
} ds18b20_state_t;

static ds18b20_state_t state = DS_IDLE;
static uint32_t ds18b20_timer;
static int32_t  last_millic;
static uint8_t  last_valid = 0;

void handle_ds18b20_poll(void) {
    switch (state) {
        case DS_IDLE:
            if ((uint16_t)(millis()-ds18b20_timer) > TICK_20S) {
                if (ow_reset()) {
                    ow_write_byte(DS18B20_CMD_SKIP_ROM);
                    ow_write_byte(DS18B20_CMD_CONVERT_T);
                    ds18b20_timer = millis();
                    state = DS_CONVERTING;
                } else {
                    ds18b20_timer = millis();   /* retry again in 30s */
                }
            }
            break;

        case DS_CONVERTING:
            /* Don't poll ow_read_bit() in a tight spin here - either
             * check it occasionally (every few main-loop passes) or
             * just wait out a fixed ~750ms via tick comparison, since
             * you don't need the earliest-possible result. */
            if ((uint16_t)(millis()-ds18b20_timer) > TICK_750MS) {
                state = DS_READING;
            }
            break;

        case DS_READING: {
            uint8_t lsb, msb;
            if (ow_reset()) {
                ow_write_byte(DS18B20_CMD_SKIP_ROM);
                ow_write_byte(DS18B20_CMD_READ_SCRATCH);
                lsb = ow_read_byte();
                msb = ow_read_byte();
                last_millic = ((int32_t)(int16_t)(((uint16_t)msb << 8) | lsb) * 625) / 10;
                modbus_set_ds18b20_temp(last_millic);
                last_valid = 1;
            }
            ds18b20_timer = millis();
            state = DS_IDLE;
            break;
        }
    }
}

uint8_t ds18b20_get_last(int32_t *millic) {
    if (!last_valid) return 0;
    *millic = last_millic;
    return 1;
}
