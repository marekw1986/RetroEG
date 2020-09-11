#include "io.h"
#include "mc6840.h"

static uint8_t port_cache = 0;


void __fastcall__ port_write (uint8_t data) {
    port_cache = data;
    PORT = port_cache;
}


void __fastcall__ port_set (uint8_t data) {
    port_cache |= data;
    PORT = port_cache;
}


void __fastcall__ port_clr (uint8_t data) {
    port_cache &= ~data;
    PORT = port_cache;
}


void __fastcall__ port_tgl (uint8_t data) {
    port_cache ^= data;
    PORT = port_cache;
}
