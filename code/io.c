#include "io.h"
#include "mc6840.h"

static uint8_t port_cache = 0;


void __fastcall__ key_init (key_t *key, uint8_t pin, void (*push_proc)(void)) {
	key->pin = pin;
	key->timer = 0;
	key->last_state = 0;
	key->push_proc = push_proc;
}


void __fastcall__ key_update (key_t *key) {
	uint8_t key_press = !(BTNS & key->pin);
	
    if (key_press != (key->last_state)) {
		if (key_press) {
			key->timer = millis();
		}
		else {
			if ( key->timer && ( (uint8_t)(millis()-(key->timer)) > SHORT_WAIT ) ) {
				if (key->push_proc) (key->push_proc)();
				key->timer = 0;
			}
		}
		key->last_state = key_press;
	}
}


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
