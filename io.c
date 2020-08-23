#include "io.h"
#include "mc6840.h"

static uint8_t port_cache = 0;


void port_write (uint8_t data) {
    port_cache = data;
    PORT = port_cache;
}


void port_set (uint8_t data) {
    port_cache |= data;
    PORT = port_cache;
}


void port_clr (uint8_t data) {
    port_cache &= ~data;
    PORT = port_cache;
}


void port_tgl (uint8_t data) {
    port_cache ^= data;
    PORT = port_cache;
}
