#include "bus8255.h"

uint8_t porta_shadow;

void bus8255_init(void) {
    CONF_8255 = CONF_8255_MODE0_A_OUT_B_IN;

    /* Idle state with INVERTING 7406 buffers on PA3/PA4/PA6:
     * writing 0 releases the driver (7406 output floats, external
     * pull-up holds the bus high); */
    porta_shadow = 0x00;
    PA_8255 = porta_shadow;
}
