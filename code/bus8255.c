#include "bus8255.h"

uint8_t porta_shadow;

void bus8255_init(void) {
    CONF_8255 = CONF_8255_MODE0_A_OUT_B_IN;

    /* Idle state: release all three open-collector drivers so the
     * external pull-ups hold 1-Wire and I2C high. */
    porta_shadow = BIT_PA_1W_OUT | BIT_PA_SDA_OUT | BIT_PA_SCL_OUT;
    PA_8255 = porta_shadow;
}
